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

static DWORD s_dwTlsIndex = TLS_OUT_OF_INDEXES;

#elif defined(__linux__)

__thread CFL_THREADP s_threadObject = THREAD_UNDEF;

#endif

#ifdef _WIN32
static void initThreadLocalStorage() {
   s_dwTlsIndex = TlsAlloc();
   if (s_dwTlsIndex != TLS_OUT_OF_INDEXES) {
      CFL_THREADP thread = malloc(sizeof(CFL_THREAD));
      if (thread != NULL) {
         memset(thread, 0, sizeof(CFL_THREAD));
         TlsSetValue(s_dwTlsIndex, thread);
      }
   }
}
#endif

CFL_THREADP cfl_thread_new(CFL_THREAD_FUNC func) {
   CFL_THREADP thread = malloc(sizeof(CFL_THREAD));
   if (thread != NULL) {
      memset(thread, 0, sizeof(CFL_THREAD));
      thread->func = func;
   }

#ifdef _WIN32
   if (s_dwTlsIndex == TLS_OUT_OF_INDEXES) {
      initThreadLocalStorage();
   }
#endif
   return thread;
}

void cfl_thread_free(CFL_THREADP thread) {
#ifdef _WIN32
   if (thread->handle) {
      CloseHandle(thread->handle);
   }
#endif
   free(thread);
}

CFL_THREADP cfl_thread_getCurrent(void) {
#ifdef _WIN32
   if (s_dwTlsIndex == TLS_OUT_OF_INDEXES) {
      initThreadLocalStorage();
   }
   return (CFL_THREADP) TlsGetValue(s_dwTlsIndex);
#elif defined(__linux__)
   return s_threadObject;
#endif
}

void * cfl_thread_getData(CFL_THREADP thread) {
   return thread->data;
}

void cfl_thread_setData(CFL_THREADP thread, void *data) {
   thread->data = data;
}

#ifdef _WIN32

static DWORD WINAPI startFunction(LPVOID param) {
   CFL_THREADP thread = (CFL_THREADP) param;
   if (s_dwTlsIndex != TLS_OUT_OF_INDEXES) {
      TlsSetValue(s_dwTlsIndex, thread);
      if (thread->func != NULL) {
         thread->func(thread->param);
      }
      return 1;
   }
   return 0;
}

#else

static void *startFunction(void *param) {
   CFL_THREADP thread = (CFL_THREADP) param;
   s_threadObject = thread;
   thread->func(thread->param);
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
#elif defined(__linux__)
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
#elif defined(__linux__)
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
#elif defined(__linux__)
   struct timespec ts;

   //
   if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
      ts.tv_sec += (int) (timeout / 1000);
      ts.tv_nsec += (long) ((timeout % 1000) * 1000000);
      return pthread_timedjoin_np(thread->handle, NULL, &ts) == 0 ? CFL_TRUE : CFL_FALSE;
   }
#endif
}

CFL_BOOL cfl_thread_sleep(CFL_UINT32 time) {
#ifdef _WIN32
   Sleep((DWORD) time);
#elif defined(__linux__)
   sleep((unsigned int) time / 1000);
#endif
   return CFL_TRUE;
}
