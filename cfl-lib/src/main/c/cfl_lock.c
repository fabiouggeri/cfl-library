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

#ifdef __linux__
#include <sys/time.h>
#else
#include <time.h>
#endif

#define SLEEP_TIMEOUT  50
#define EXCLUSIVE_LOCK 0xFFFFFFFF

#ifdef __linux__
   #define GET_CURRENT_THREAD_ID   pthread_self()
   #define IS_LOCKED_BY_ME(l)      pthread_equal((l)->threadId, GET_CURRENT_THREAD_ID)
   #define ERROR_WAITING(t)        (t != 0)
   #define TIMEOUTED(t)            (t == ETIMEDOUT)
#else
   #define GET_CURRENT_THREAD_ID   GetCurrentThreadId()
   #define IS_LOCKED_BY_ME(l)      ((l)->threadId == GET_CURRENT_THREAD_ID)
   #define ERROR_WAITING(t)        (t == 0)
   #define TIMEOUTED(t)            (t == 0 && GetLastError() == ERROR_TIMEOUT)
#endif

#if defined(__linux__) || ! defined(_CFL_CONDITION_VAR)
   #define WAIT_RESULT int
#elif defined(_WIN32)
   #define WAIT_RESULT BOOL
#else
   #define WAIT_RESULT int
#endif

#ifdef _WIN32
   #ifdef SRWLOCK_INIT
      #define INITIALIZE_LOCK_HANDLE(l)    InitializeSRWLock(&((l)->handle))
      #define RELEASE_LOCK_HANDLE(l)       // do nothing
      #define ACQUIRE_EXCLUSIVE(l)         AcquireSRWLockExclusive(&((l)->handle))
      #define RELEASE_EXCLUSIVE(l)         ReleaseSRWLockExclusive(&((l)->handle))
      #define SLEEP_CONDITION(l)           SleepConditionVariableSRW(&((l)->lockChange), &((l)->handle), (l)->timeout, 0)
      #define SLEEP_CONDITION_VAR(l, v, t) SleepConditionVariableSRW(v, &((l)->handle), t, 0)
      #define INITIALIZE_CONDITION_VAR(v)  InitializeConditionVariable(v)
      #define RELEASE_CONDITION_VAR(l)     // do nothing
      #define WAKE_ALL_CONDITION(v)        WakeAllConditionVariable(v)
      #define WAKE_CONDITION(v)            WakeConditionVariable(v)
   #else
      #define INITIALIZE_LOCK_HANDLE(l)   InitializeCriticalSection(&((l)->handle))
      #define RELEASE_LOCK_HANDLE(l)      DeleteCriticalSection(&((l)->handle))
      #define ACQUIRE_EXCLUSIVE(l)        EnterCriticalSection(&((l)->handle))
      #define RELEASE_EXCLUSIVE(l)        LeaveCriticalSection(&((l)->handle))
      #ifdef _CFL_CONDITION_VAR
         #define SLEEP_CONDITION(l)           waitCondition(l, &((l)->lockChange), (l)->timeout)
         #define SLEEP_CONDITION_VAR(l, v, t) waitCondition(l, v, t)
         #define INITIALIZE_CONDITION_VAR(v)  InitializeConditionVariable(v)
         #define RELEASE_CONDITION_VAR(l)     // do nothing
         #define WAKE_ALL_CONDITION(v)        WakeAllConditionVariable(v)
         #define WAKE_CONDITION(v)            WakeConditionVariable(v)
      #else
         #define SLEEP_CONDITION(l)           waitCondition(l, &((l)->lockChange), (l)->timeout)
         #define SLEEP_CONDITION_VAR(l, v, t) waitCondition(l, v, t)
         #define INITIALIZE_CONDITION_VAR(v)  (*v = 0)
         #define RELEASE_CONDITION_VAR(l)     // do nothing
         #define WAKE_ALL_CONDITION(v)        (++(*v))
         #define WAKE_CONDITION(v)            (++(*v))
      #endif
   #endif
#elif defined(__linux__)
   #define INITIALIZE_LOCK_HANDLE(l)    pthread_mutex_init(&((l)->handle), 0)
   #define RELEASE_LOCK_HANDLE(l)       pthread_mutex_destroy(&((l)->handle))
   #define ACQUIRE_EXCLUSIVE(l)         pthread_mutex_lock(&((l)->handle))
   #define RELEASE_EXCLUSIVE(l)         pthread_mutex_unlock(&((l)->handle))
   #define SLEEP_CONDITION(l)           waitCondition(l, &((l)->lockChange), (l)->timeout)
   #define SLEEP_CONDITION_VAR(l, v, t) waitCondition(l, &((l)->lockChange), (l)->timeout)
   #define INITIALIZE_CONDITION_VAR(v)  pthread_cond_init(v, 0)
   #define RELEASE_CONDITION_VAR(l)     pthread_cond_destroy(l)
   #define WAKE_ALL_CONDITION(v)        pthread_cond_broadcast(v)
   #define WAKE_CONDITION(v)            pthread_cond_signal(v)

#endif

static int waitCondition(CFL_LOCKP pLock, CFL_CONDITION_VARIABLEP pVar, CFL_UINT32 timeout) {
#if defined(__linux__)
   struct timespec ts;
   struct timeval tv;
   CFL_UINT64 nanoseconds;

   if (timeout == CFL_WAIT_FOREVER) {
      return pthread_cond_wait(pVar, &pLock->handle);
   } else {
      gettimeofday(&tv, NULL);

      nanoseconds = ((CFL_UINT64) tv.tv_sec) * 1000 * 1000 * 1000 +
              timeout * 1000 * 1000 +
              ((CFL_UINT64) tv.tv_usec) * 1000;

      ts.tv_sec = nanoseconds / 1000 / 1000 / 1000;
      ts.tv_nsec = (long) (nanoseconds - ((CFL_UINT64) ts.tv_sec) * 1000 * 1000 * 1000);
      return pthread_cond_timedwait(pVar, &pLock->handle, &ts);
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
      } else if (*pVar > 0) {
         *pVar = 0;
         break;
      }
   }
   return 1;
#else
   return 1;
#endif
}

/*
 * TODO: create a way to relase functions unlock correspondent lock request.
 *    Ex. acquire EXCLUSIVE, acquire SHARED, release SHARED, release EXCLUSIVE
 * TODO: create cfl_lock_tryExclusive and cfl_lock_tryShared
 */
CFL_LOCKP cfl_lock_new(void) {
   CFL_LOCKP lock = malloc(sizeof(CFL_LOCK));
   cfl_lock_init(lock);
   lock->isAllocated = CFL_TRUE;
   return lock;
}

void cfl_lock_free(CFL_LOCKP pLock) {
   if (pLock && pLock->isAllocated) {
      RELEASE_LOCK_HANDLE(pLock);
      RELEASE_CONDITION_VAR(&pLock->lockChange);
      free(pLock);
   }
}

void cfl_lock_init(CFL_LOCKP pLock) {
   INITIALIZE_LOCK_HANDLE(pLock);
   INITIALIZE_CONDITION_VAR(&pLock->lockChange);
   pLock->lockCount = 0;
   pLock->threadId = 0;
   pLock->timeout = CFL_WAIT_FOREVER;
   pLock->isAllocated = CFL_FALSE;
}

static CFL_BOOL registerLock(CFL_LOCKP pLock, CFL_BOOL bExclusive) {
   if (bExclusive) {
      if (pLock->lockCount == 0 || IS_LOCKED_BY_ME(pLock)) {
         pLock->lockCount = EXCLUSIVE_LOCK;
         pLock->threadId = GET_CURRENT_THREAD_ID;
         return CFL_TRUE;
      }
   } else if (pLock->lockCount != EXCLUSIVE_LOCK) {
      ++pLock->lockCount;
      return CFL_TRUE;
   } else if (IS_LOCKED_BY_ME(pLock)) {
      return CFL_TRUE;;
   }
   return CFL_FALSE;
}

static CFL_BOOL unregisterLock(CFL_LOCKP pLock) {
   if (pLock->lockCount == EXCLUSIVE_LOCK) {
      if (IS_LOCKED_BY_ME(pLock)) {
         pLock->lockCount = 0;
         pLock->threadId = 0;
         return CFL_TRUE;
      }
   } else if (pLock->lockCount > 0) {
      --(pLock->lockCount);
      return CFL_TRUE;
   }
   return CFL_FALSE;
}

CFL_BOOL cfl_lock_tryExclusive(CFL_LOCKP pLock) {
   if (pLock) {
      CFL_BOOL bLocked;
      ACQUIRE_EXCLUSIVE(pLock);
      bLocked = registerLock(pLock, CFL_TRUE);
      RELEASE_EXCLUSIVE(pLock);
      WAKE_ALL_CONDITION(&pLock->lockChange);
      return bLocked;
   }
   return CFL_FALSE;
}

void cfl_lock_acquireExclusive(CFL_LOCKP pLock) {
   if (pLock) {
      CFL_BOOL bLocked;
      ACQUIRE_EXCLUSIVE(pLock);
      do {
         bLocked = registerLock(pLock, CFL_TRUE);
         if (! bLocked) {
            SLEEP_CONDITION(pLock);
         }
      } while (! bLocked);
      RELEASE_EXCLUSIVE(pLock);
      WAKE_ALL_CONDITION(&pLock->lockChange);
   }
}

CFL_BOOL cfl_lock_tryShared(CFL_LOCKP pLock) {
   if (pLock) {
      CFL_BOOL bLocked;
      ACQUIRE_EXCLUSIVE(pLock);
      bLocked = registerLock(pLock, CFL_FALSE);
      RELEASE_EXCLUSIVE(pLock);
      WAKE_ALL_CONDITION(&pLock->lockChange);
      return bLocked;
   }
   return CFL_FALSE;
}

void cfl_lock_acquireShared(CFL_LOCKP pLock) {
   if (pLock) {
      CFL_BOOL bLocked;
      ACQUIRE_EXCLUSIVE(pLock);
      do {
         bLocked = registerLock(pLock, CFL_FALSE);
         if (! bLocked) {
            SLEEP_CONDITION(pLock);
         }
      } while (! bLocked);
      RELEASE_EXCLUSIVE(pLock);
      WAKE_ALL_CONDITION(&pLock->lockChange);
   }
}

void cfl_lock_release(CFL_LOCKP pLock) {
   if (pLock) {
      ACQUIRE_EXCLUSIVE(pLock);
      unregisterLock(pLock);
      RELEASE_EXCLUSIVE(pLock);
      WAKE_ALL_CONDITION(&pLock->lockChange);
   }
}

CFL_CONDITION_VARIABLEP cfl_lock_conditionVar(void) {
   CFL_CONDITION_VARIABLEP pVar = (CFL_CONDITION_VARIABLEP) malloc(sizeof(CFL_CONDITION_VARIABLE));
   if (pVar != NULL) {
      INITIALIZE_CONDITION_VAR(pVar);
   }
   return pVar;
}

void cfl_lock_releaseConditionVar(CFL_CONDITION_VARIABLEP pVar) {
   if (pVar != NULL) {
      RELEASE_CONDITION_VAR(pVar);
   }
}

CFL_BOOL cfl_lock_waitCondition(CFL_LOCKP pLock, CFL_CONDITION_VARIABLEP pVar) {
   CFL_BOOL bSuccess = CFL_FALSE;
   if (pLock) {
      CFL_BOOL bExclusive;

      ACQUIRE_EXCLUSIVE(pLock);
      bExclusive = pLock->lockCount == EXCLUSIVE_LOCK;
      if (unregisterLock(pLock)) {
         SLEEP_CONDITION_VAR(pLock, pVar, CFL_WAIT_FOREVER);
         registerLock(pLock, bExclusive);
         bSuccess = CFL_TRUE;
      }
      RELEASE_EXCLUSIVE(pLock);
   }
   return bSuccess;
}

CFL_UINT8 cfl_lock_waitConditionTimeout(CFL_LOCKP pLock, CFL_CONDITION_VARIABLEP pVar, CFL_UINT32 timeout) {
   CFL_UINT8 res = CFL_LOCK_ERROR;
   if (pLock) {
      CFL_BOOL bExclusive;

      ACQUIRE_EXCLUSIVE(pLock);
      bExclusive = pLock->lockCount == EXCLUSIVE_LOCK;
      if (unregisterLock(pLock)) {
         WAIT_RESULT waitRes = SLEEP_CONDITION_VAR(pLock, pVar, timeout);
         registerLock(pLock, bExclusive);
         if (! ERROR_WAITING(waitRes)) {
            res = CFL_LOCK_SUCCESS;
         } else if (TIMEOUTED(waitRes)) {
            res = CFL_LOCK_TIMEOUT;
         }
      }
      RELEASE_EXCLUSIVE(pLock);
   }
   return res;
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
#ifdef __linux__
   return errno;
#else
   return GetLastError();
#endif
}
