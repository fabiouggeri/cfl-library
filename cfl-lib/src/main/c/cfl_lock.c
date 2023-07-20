/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <stdlib.h>

#include "cfl_lock.h"
#include "cfl_hash.h"

#if defined(CFL_OS_LINUX)
#include <sys/time.h>
#else
#include <time.h>
#endif

#define SLEEP_TIMEOUT  10

#define IS_EXCLUSIVE_LOCK(l)       ((l)->lockOwner != NULL_THREAD)

#if defined(CFL_OS_LINUX) || ! defined(_CFL_CONDITION_VAR)
   #define WAIT_RESULT int
#elif defined(CFL_OS_WINDOWS)
   #define WAIT_RESULT BOOL
#else
   #define WAIT_RESULT int
#endif

#if defined(CFL_OS_WINDOWS)

   #define GET_CURRENT_THREAD_ID      GetCurrentThreadId()
   #define IS_LOCKED_BY_ME(l)         ((l)->lockOwner == GET_CURRENT_THREAD_ID)
   #define ERROR_WAITING_CONDITION(t) (t == 0)
   #define TIMEOUT_WAITING(t)         (t == 0 && GetLastError() == ERROR_TIMEOUT)
   #define NULL_THREAD                0

   #ifdef SRWLOCK_INIT

      #define INITIALIZE_LOCK_HANDLE(l)           InitializeSRWLock(&((l)->handle))
      #define RELEASE_LOCK_HANDLE(l)              // do nothing
      #define ACQUIRE_LOCK(l)                     AcquireSRWLockExclusive(&((l)->handle))
      #define RELEASE_LOCK(l)                     ReleaseSRWLockExclusive(&((l)->handle))
      
      #define INITIALIZE_CONDITION_HANDLE(v)      InitializeConditionVariable(&((v)->handle))
      #define WAIT_CONDITION(l, v, t)             SleepConditionVariableSRW(&((v)->handle), &((l)->handle), t, 0)
      #define RELEASE_CONDITION_HANDLE(v)         // do nothing
      #define WAKE_ALL_CONDITION(v)               WakeAllConditionVariable(&((v)->handle))
      #define WAKE_CONDITION(v)                   WakeConditionVariable(&((v)->handle))

   #else

      #define INITIALIZE_LOCK_HANDLE(l)           InitializeCriticalSection(&((l)->handle))
      #define RELEASE_LOCK_HANDLE(l)              DeleteCriticalSection(&((l)->handle))
      #define ACQUIRE_LOCK(l)                     EnterCriticalSection(&((l)->handle))
      #define RELEASE_LOCK(l)                     LeaveCriticalSection(&((l)->handle))

      #ifdef _CFL_CONDITION_VAR

         #define INITIALIZE_CONDITION_HANDLE(v)   InitializeConditionVariable(&((v)->handle))
         #define WAIT_CONDITION(l, v, t)          SleepConditionVariableCS(&((v)->handle), &((l)->handle), t)
         #define RELEASE_CONDITION_HANDLE(v)      // do nothing
         #define WAKE_ALL_CONDITION(v)            WakeAllConditionVariable(&((v)->handle))
         #define WAKE_CONDITION(v)                WakeConditionVariable(&((v)->handle))

      #else

         #define INITIALIZE_CONDITION_HANDLE(v)  ((v)->handle = 0)
         #define WAIT_CONDITION(l, v, t)          waitCondition(l, v, t)
         #define RELEASE_CONDITION_HANDLE(v)      // do nothing
         #define WAKE_ALL_CONDITION(v)            (++((v)->handle))
         #define WAKE_CONDITION(v)                (++((v)->handle))
         #define WAIT_FUNCTION

      #endif

   #endif

#elif defined(CFL_OS_LINUX)

   #define GET_CURRENT_THREAD_ID      pthread_self()
   #define IS_LOCKED_BY_ME(l)         pthread_equal((l)->lockOwner, GET_CURRENT_THREAD_ID)
   #define ERROR_WAITING_CONDITION(t) (t != 0)
   #define TIMEOUT_WAITING(t)         (t == ETIMEDOUT)
   #define NULL_THREAD                ((pthread_t)0)

   #define INITIALIZE_LOCK_HANDLE(l)              pthread_mutex_init(&((l)->handle), 0)
   #define RELEASE_LOCK_HANDLE(l)                 pthread_mutex_destroy(&((l)->handle))
   #define ACQUIRE_LOCK(l)                        pthread_mutex_lock(&((l)->handle))
   #define RELEASE_LOCK(l)                        pthread_mutex_unlock(&((l)->handle))
   #define INITIALIZE_CONDITION_HANDLE(v)         pthread_cond_init(&((v)->handle), 0)
   #define WAIT_CONDITION(l, v, t)                waitCondition(l, v, t)
   #define RELEASE_CONDITION_HANDLE(v)            pthread_cond_destroy(&((v)->handle))
   #define WAKE_ALL_CONDITION(v)                  pthread_cond_broadcast(&((v)->handle))
   #define WAKE_CONDITION(v)                      pthread_cond_signal(&((v)->handle))
   #define WAIT_FUNCTION

#endif

#ifdef WAIT_FUNCTION

static int waitCondition(CFL_LOCKP pLock, CFL_CONDITION_VARIABLEP pVar, CFL_UINT32 timeout) {
#if defined(CFL_OS_LINUX)
   struct timeval tv;

   if (timeout == CFL_WAIT_FOREVER) {
      return pthread_cond_wait(&pVar->handle, &pLock->handle);
   } else {
      struct timespec ts;
      clock_gettime(CLOCK_REALTIME, &ts);
      ts.tv_sec  += timeout / 1000;
      ts.tv_nsec += (timeout % 1000) * 1000000;
      if (ts.tv_nsec > 1000000000) {
         ts.tv_sec  += 1;
         ts.tv_nsec -= 1000000000;
      }
      return pthread_cond_timedwait(&pVar->handle, &pLock->handle, &ts);
   }
#elif ! defined(_CFL_CONDITION_VAR)
   clock_t lastClock;
   double elapsed = 0.0;
   while (1) {
      LeaveCriticalSection(&pLock->handle);
      lastClock = clock();
      Sleep(SLEEP_TIMEOUT);
      EnterCriticalSection(&pLock->handle);
      elapsed += (((double)clock() - lastClock) / CLOCKS_PER_SEC) * 1000;
      if (((CFL_UINT32)elapsed) > timeout) {
         return 0;
      } else if (pVar->handle > 0) {
         pVar->handle = 0;
         break;
      }
   }
   return 1;
#else
   return 1;
#endif
}
#endif

CFL_LOCKP cfl_lock_new(void) {
   CFL_LOCKP lock = malloc(sizeof(CFL_LOCK));
   cfl_lock_init(lock);
   lock->isAllocated = CFL_TRUE;
   return lock;
}

void cfl_lock_init(CFL_LOCKP pLock) {
   INITIALIZE_LOCK_HANDLE(pLock);
   pLock->isAllocated = CFL_FALSE;
}

void cfl_lock_free(CFL_LOCKP pLock) {
   if (pLock) {
      RELEASE_LOCK_HANDLE(pLock);
      if (pLock->isAllocated) {
         free(pLock);
      }
   }
}

void cfl_lock_acquire(CFL_LOCKP pLock) {
   if (pLock) {
      ACQUIRE_LOCK(pLock);
   }
}

void cfl_lock_release(CFL_LOCKP pLock) {
   if (pLock) {
      RELEASE_LOCK(pLock);
   }
}

void cfl_lock_initConditionVar(CFL_CONDITION_VARIABLEP pVar) {
   if (pVar) {
      INITIALIZE_CONDITION_HANDLE(pVar);
      pVar->isAllocated = CFL_FALSE;
   }
}

CFL_CONDITION_VARIABLEP cfl_lock_newConditionVar(void) {
   CFL_CONDITION_VARIABLEP pVar = (CFL_CONDITION_VARIABLEP) malloc(sizeof(CFL_CONDITION_VARIABLE));
   if (pVar) {
      cfl_lock_initConditionVar(pVar);
      pVar->isAllocated = CFL_TRUE;
   }
   return pVar;
}

void cfl_lock_freeConditionVar(CFL_CONDITION_VARIABLEP pVar) {
   if (pVar) {
      RELEASE_CONDITION_HANDLE(pVar);
      if (pVar->isAllocated) {
         free(pVar);
      }
   }
}

CFL_BOOL cfl_lock_conditionWait(CFL_LOCKP pLock, CFL_CONDITION_VARIABLEP pVar) {
   if (pLock && pVar) {
      WAIT_CONDITION(pLock, pVar, CFL_WAIT_FOREVER);
      return CFL_TRUE;
   }
   return CFL_FALSE;
}

CFL_UINT8 cfl_lock_conditionWaitTimeout(CFL_LOCKP pLock, CFL_CONDITION_VARIABLEP pVar, CFL_UINT32 timeout) {
   if (pLock && pVar) {
      CFL_UINT8 res;
      WAIT_RESULT waitRes;
      waitRes = WAIT_CONDITION(pLock, pVar, timeout);
      if (TIMEOUT_WAITING(waitRes)) {
         res = CFL_LOCK_TIMEOUT;
      } else if (ERROR_WAITING_CONDITION(waitRes)) {
         res = CFL_LOCK_ERROR;
      } else {
         res = CFL_LOCK_SUCCESS;
      }
      return res;
   }
   return CFL_LOCK_ERROR;
}

void cfl_lock_conditionWake(CFL_CONDITION_VARIABLEP pVar) {
   if (pVar != NULL) {
      WAKE_CONDITION(pVar);
   }
}

void cfl_lock_conditionWakeAll(CFL_CONDITION_VARIABLEP pVar) {
   if (pVar != NULL) {
      WAKE_ALL_CONDITION(pVar);
   }
}

CFL_INT32 cfl_lock_lastErrorCode(void) {
#if defined(CFL_OS_LINUX)
   return errno;
#else
   return GetLastError();
#endif
}
