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

#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>

#include "cfl_thread.h"
#include "cfl_lock.h"

#ifdef _WIN32

static volatile LONG s_threadStoreInitialized = 0;
static DWORD s_threadStorageKey = TLS_OUT_OF_INDEXES;

#define INIT_THREAD_STORAGE   if (s_threadStoreInitialized == 0) {\
                                 if (InterlockedCompareExchange(&s_threadStoreInitialized, 1, 0) == 0) {\
                                    s_threadStorageKey = TlsAlloc();\
                                 }\
                              }

#define GET_THREAD(t) INIT_THREAD_STORAGE \
                      t = (CFL_THREADP) TlsGetValue(s_threadStorageKey)

#define SET_THREAD(t) INIT_THREAD_STORAGE \
                      TlsSetValue(s_threadStorageKey, t)

#else

static pthread_once_t key_once = PTHREAD_ONCE_INIT;

static int s_threadStoreInitialized = 0;

static pthread_key_t s_threadStorageKey;

#define INIT_THREAD_STORAGE   if (! s_threadStoreInitialized) {\
                                 pthread_once(&key_once, init_key);\
                              }

#define GET_THREAD(t) INIT_THREAD_STORAGE \
                      t = (CFL_THREADP) pthread_getspecific(s_threadStorageKey)

#define SET_THREAD(t) INIT_THREAD_STORAGE \
                      pthread_setspecific(s_threadStorageKey, t)

static void freeStoredData(void *data) {
   CFL_THREADP thread = (CFL_THREADP)data;
   if (thread != NULL && ! thread->manualAllocation) {
      if (thread->exitCallback != NULL) {
         thread->exitCallback(thread);
      }
      cfl_thread_free(thread);
   }
}

static void init_key(void) {
   pthread_key_create(&s_threadStorageKey, freeStoredData);
   s_threadStoreInitialized = 1;
}

#endif

static CFL_THREADP initCurrentThread(void) {
   CFL_THREADP thread = malloc(sizeof(CFL_THREAD));
   if (thread == NULL) {
      return NULL;
   }
   memset(thread, 0, sizeof(CFL_THREAD));
   thread->manualAllocation = CFL_FALSE;
   SET_THREAD(thread);
   return thread;
}

CFL_THREADP cfl_thread_newOptions(CFL_THREAD_FUNC func, CFL_THREAD_CALLBACK exitCallback) {
   CFL_THREADP thread = malloc(sizeof(CFL_THREAD));
   if (thread == NULL) {
      return NULL;
   }

   memset(thread, 0, sizeof(CFL_THREAD));
   thread->manualAllocation = CFL_TRUE;
   thread->func = func;
   thread->exitCallback = exitCallback;

   return thread;
}

CFL_THREADP cfl_thread_new(CFL_THREAD_FUNC func) {
   CFL_THREADP thread = malloc(sizeof(CFL_THREAD));
   if (thread == NULL) {
      return NULL;
   }

   memset(thread, 0, sizeof(CFL_THREAD));
   thread->manualAllocation = CFL_TRUE;
   thread->func = func;

   return thread;
}

void cfl_thread_free(CFL_THREADP thread) {
   SET_THREAD(NULL);
#ifdef _WIN32
   CloseHandle(thread->handle);
#else
   pthread_detach(thread->handle);
#endif
   free(thread);
}

CFL_THREADP cfl_thread_getCurrent(void) {
   CFL_THREADP thread;
   
   GET_THREAD(thread);
   if (thread == NULL) {
      thread = initCurrentThread();
   }
   return thread;
}

void * cfl_thread_getData(CFL_THREADP thread) {
   return thread->data;
}

void cfl_thread_setData(CFL_THREADP thread, void *data) {
   thread->data = data;
}

void cfl_thread_setExitCallback(CFL_THREADP thread, CFL_THREAD_CALLBACK exitCallback) {
   thread->exitCallback = exitCallback;
}

#ifdef _WIN32

static DWORD WINAPI startFunction(LPVOID param) {
   CFL_THREADP thread = (CFL_THREADP) param;
   SET_THREAD(thread);
   if (thread->func != NULL) {
      thread->func(thread->param);
   }
   return 1;
}

#else

static void *startFunction(void *param) {
   CFL_THREADP thread = (CFL_THREADP) param;
   SET_THREAD(thread);
   if (thread->func != NULL) {
      thread->func(thread->param);
   }
   return NULL;
}
#endif

CFL_BOOL cfl_thread_start(CFL_THREADP thread, void * param) {
#ifdef _WIN32
   if (thread->handle == NULL) {
      DWORD threadId;
      thread->param = param;
      thread->handle = CreateThread(NULL, 0, startFunction, thread, 0, &threadId);
      return thread->handle != NULL ? CFL_TRUE : CFL_FALSE;
   }
   return CFL_FALSE;
#else
   thread->param = param;
   return pthread_create(&thread->handle, NULL, startFunction, thread) == 0 ? CFL_TRUE : CFL_FALSE;
#endif
}

CFL_BOOL cfl_thread_wait(CFL_THREADP thread) {
#ifdef _WIN32
   if (thread->handle) {
      return WaitForSingleObject(thread->handle, INFINITE) == WAIT_OBJECT_0 ? CFL_TRUE : CFL_FALSE;
   }
   return CFL_FALSE;
#else
   return pthread_join(thread->handle, NULL) == 0 ? CFL_TRUE : CFL_FALSE;
#endif
}

CFL_BOOL cfl_thread_waitTimeout(CFL_THREADP thread, CFL_INT32 timeout) {
#ifdef _WIN32
   if (thread->handle) {
      DWORD res = WaitForSingleObject(thread->handle, timeout);
      return res == WAIT_OBJECT_0 || res == WAIT_TIMEOUT ? CFL_TRUE : CFL_FALSE;
   }
   return CFL_FALSE;
#else
   struct timespec ts;

   //
   if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
      ts.tv_sec += (int) (timeout / 1000);
      ts.tv_nsec += (long) ((timeout % 1000) * 1000000);
      return pthread_timedjoin_np(thread->handle, NULL, &ts) == 0 ? CFL_TRUE : CFL_FALSE;
   }
#endif
}

CFL_BOOL cfl_thread_kill(CFL_THREADP thread) {
#ifdef _WIN32
   return TerminateThread(thread->handle, 1) ? CFL_TRUE : CFL_FALSE;
#else
   return pthread_cancel(thread->handle) == 0 ? CFL_TRUE : CFL_FALSE;
#endif
}

CFL_BOOL cfl_thread_sleep(CFL_UINT32 time) {
#ifdef _WIN32
   Sleep((DWORD) time);
#else
   usleep((useconds_t) time * 1000);
#endif
   return CFL_TRUE;
}
