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
#include "cfl_atomic.h"

#if defined(CFL_THREAD_WINRAWAPI)
   #define THREAD_STARTFUNC(f) static DWORD WINAPI f(LPVOID param)
   #define THREAD_END(r)       return r;
#elif defined(CFL_OS_WINDOWS)
   #include <process.h>

   #define THREAD_STARTFUNC(f) static unsigned __stdcall f(void *param)
   #define THREAD_END(r)       _endthreadex(r); return r;
#else
   #define THREAD_STARTFUNC(f) static void* f(void *param)
   #define THREAD_END(r)       return NULL;
#endif

#define DEFINE_GET_SET(datatype, typename, defaultValue) \
   datatype cfl_thread_varGet##typename(CFL_THREAD_VARIABLEP threadVar) { \
      CFL_THREAD_VAR_DATA *varData = thread_dataGet(threadVar);\
      return varData != NULL ? *((datatype *) &varData->data[0]) : defaultValue;\
   }\
   CFL_BOOL cfl_thread_varSet##typename(CFL_THREAD_VARIABLEP threadVar, datatype data) {\
      CFL_THREAD_VAR_DATA *varData = thread_dataGet(threadVar);\
      if (varData == NULL) {\
         return CFL_FALSE;\
      }\
      *((datatype *) &varData->data[0]) = data;\
      return CFL_TRUE;\
   }

#define DATA_SIZE(d) ((d)->dataSize > 0 ? (d)->dataSize : sizeof(VAR_DATA))

/*
#if defined( _MSC_VER ) && ( _MSC_VER > 1500 )
   #define CFL_THREAD_DATA __declspec( thread )
#elif ( defined( __GNUC__ ) && __GNUC__ >= 3 ) && defined( CFL_OS_LINUX )
   #define CFL_THREAD_DATA      __thread
#endif

*/

static CFL_INT32 s_thredCount = 0;
/***********
 * WINDOWS *
 ***********/
#if defined(CFL_OS_WINDOWS)

typedef HRESULT (WINAPI *TSetThreadDescription)(HANDLE, PCWSTR);

static volatile LONG s_threadStoreInitialized = 0;
static DWORD s_threadStorageKey = TLS_OUT_OF_INDEXES;

static CFL_BOOL s_procAddressLoaded = CFL_FALSE;
static volatile TSetThreadDescription s_SetThreadDescriptionPtr = NULL;

#define GET_CURRENT_THREAD_ID GetCurrentThreadId()

#define THREAD_EQUALS(th1, th2) (th1 == th2)

#define INIT_DATA_STORAGE(d)  if (InterlockedCompareExchange(&d->initialized, 1, 0) == 0) {\
                                 d->storageKey = TlsAlloc();\
                              }

#define GET_DATA(d) TlsGetValue(d->storageKey)

#define SET_DATA(d, v) TlsSetValue(d->storageKey, v)

#define INIT_THREAD_STORAGE   if (InterlockedCompareExchange(&s_threadStoreInitialized, 1, 0) == 0) {\
                                 s_threadStorageKey = TlsAlloc();\
                              }

#define GET_THREAD() ((CFL_THREADP) TlsGetValue(s_threadStorageKey))

#define SET_THREAD(t) TlsSetValue(s_threadStorageKey, t)

#define KILL_THREAD(t) (TerminateThread((t)->handle, 1))

static TSetThreadDescription getSetThreadDescriptionPtr(void) {
  HMODULE hKernel32;
  if (s_SetThreadDescriptionPtr != NULL || s_procAddressLoaded) {
      return s_SetThreadDescriptionPtr;
  }
  hKernel32 = GetModuleHandleA("KernelBase.dll");
  if (hKernel32 == NULL) {
    return NULL;
  }
  s_SetThreadDescriptionPtr = (TSetThreadDescription) GetProcAddress(hKernel32, "SetThreadDescription");
  s_procAddressLoaded = CFL_TRUE;
  return s_SetThreadDescriptionPtr;
}

/*********
 * POSIX *
 *********/
#else

static int s_threadStoreInitialized = 0;
static pthread_key_t s_threadStorageKey;

#define GET_CURRENT_THREAD_ID pthread_self()

#define THREAD_EQUALS(th1, th2) pthread_equal(th1, th2)

#define INIT_DATA_STORAGE(d)  if (__sync_val_compare_and_swap(&d->initialized, 0, 1) == 0) {\
                                 pthread_key_create(&d->storageKey, freeVarData);\
                              }

#define GET_DATA(d) pthread_getspecific(d->storageKey)

#define SET_DATA(d, v) pthread_setspecific(d->storageKey, v)


#define INIT_THREAD_STORAGE   if (__sync_val_compare_and_swap(&s_threadStoreInitialized, 0, 1) == 0) {\
                                 pthread_key_create(&s_threadStorageKey, freeOwnData);\
                              }

#define GET_THREAD() ((CFL_THREADP) pthread_getspecific(s_threadStorageKey))

#define SET_THREAD(t) pthread_setspecific(s_threadStorageKey, t)

#define KILL_THREAD(t) (pthread_kill((t)->handle, SIGUSR1) == 0)

static void freeOwnData(void *data) {
   CFL_THREADP thread = (CFL_THREADP)data;
   if (thread != NULL && ! thread->manualAllocation) {
      cfl_thread_free(thread);
   }
}

static void freeVarData(void *data) {
   CFL_THREAD_VAR_DATA *varData = (CFL_THREAD_VAR_DATA *)data;
   if (varData != NULL && varData->var->freeData != NULL) {
      varData->var->freeData(&varData->data[0]);
   }
}

#endif

static void setDescription(CFL_THREADP thread) {
   #ifdef CFL_OS_WINDOWS
      TSetThreadDescription setDescFunction = getSetThreadDescriptionPtr();
      if (setDescFunction != NULL) {
         CFL_UINT32 len = cfl_str_length(&thread->description);
         wchar_t *threadDescription;
         threadDescription = CFL_MEM_ALLOC(sizeof(wchar_t) * (len + 1));
         MultiByteToWideChar(0, 0, cfl_str_getPtr(&thread->description), len, threadDescription, len);
         setDescFunction(thread->handle, threadDescription);
         CFL_MEM_FREE(threadDescription);
      }
   #else
      pthread_setname_np(thread->handle, cfl_str_getPtr(&thread->description));
   #endif
}

static CFL_THREADP initCurrentThread(void) {
   CFL_THREADP thread = CFL_MEM_ALLOC(sizeof(CFL_THREAD));
   CFL_INT64 threadNum;
   if (thread == NULL) {
      return NULL;
   }
   INIT_THREAD_STORAGE
   memset(thread, 0, sizeof(CFL_THREAD));
   thread->manualAllocation = CFL_FALSE;
   thread->joined = CFL_FALSE;
   thread->status = CFL_THREAD_RUNNING;
   threadNum = cfl_atomic_addInt32(&s_thredCount, 1) + 1;
   cfl_str_init(&thread->description);
   cfl_str_setFormat(&thread->description, "CFL Thread %ld", threadNum);
   setDescription(thread);
   SET_THREAD(thread);
   return thread;
}

CFL_THREADP cfl_thread_new(CFL_THREAD_FUNC func) {
   CFL_THREADP thread = CFL_MEM_ALLOC(sizeof(CFL_THREAD));
   CFL_INT64 threadNum;
   if (thread == NULL) {
      return NULL;
   }
   memset(thread, 0, sizeof(CFL_THREAD));
   thread->manualAllocation = CFL_TRUE;
   thread->joined = CFL_FALSE;
   thread->func = func;
   thread->status = CFL_THREAD_CREATED;
   threadNum = cfl_atomic_addInt32(&s_thredCount, 1) + 1;
   cfl_str_init(&thread->description);
   cfl_str_setFormat(&thread->description, "CFL Thread %ld", threadNum);
   return thread;
}

CFL_THREADP cfl_thread_newWithDescription(CFL_THREAD_FUNC func, const char *description) {
   CFL_THREADP thread = CFL_MEM_ALLOC(sizeof(CFL_THREAD));
   if (thread == NULL) {
      return NULL;
   }
   memset(thread, 0, sizeof(CFL_THREAD));
   thread->manualAllocation = CFL_TRUE;
   thread->joined = CFL_FALSE;
   thread->func = func;
   thread->status = CFL_THREAD_CREATED;
   cfl_str_initValue(&thread->description, description);
   return thread;
}

void cfl_thread_setDescription(CFL_THREADP thread, const char *description) {
   cfl_str_setValue(&thread->description, description);
   if (thread->status == CFL_THREAD_RUNNING) {
      setDescription(thread);
   }
}


void cfl_thread_free(CFL_THREADP thread) {
   if (thread == NULL) {
      return;
   }
   INIT_THREAD_STORAGE
   SET_THREAD(NULL);
#if defined(CFL_OS_WINDOWS)
   CloseHandle(thread->handle);
#else
   if (! thread->joined) { 
      pthread_detach(thread->handle);
   }
#endif
   CFL_MEM_FREE(thread);
}

CFL_BOOL cfl_thread_equals(CFL_THREAD_ID th1, CFL_THREAD_ID th2) {
   return THREAD_EQUALS(th1, th2);
}

CFL_THREAD_ID cfl_thread_id(void) {
   return GET_CURRENT_THREAD_ID;
}

CFL_THREADP cfl_thread_getCurrent(void) {
   CFL_THREADP thread;
   
   INIT_THREAD_STORAGE
   thread = GET_THREAD();
   if (thread == NULL) {
      thread = initCurrentThread();
   }
   return thread;
}

THREAD_STARTFUNC( startFunction ) {
   if (param != NULL) {
      CFL_THREADP thread = (CFL_THREADP) param;
      setDescription(thread);
      thread->status = CFL_THREAD_RUNNING;
      INIT_THREAD_STORAGE
      SET_THREAD(thread);
      if (thread->func != NULL) {
         thread->func(thread->param);
      }
      thread->status = CFL_THREAD_FINISHED;
      THREAD_END(1)
   } else {
      THREAD_END(0)
   }
}

CFL_BOOL cfl_thread_start(CFL_THREADP thread, void * param) {
#if defined(CFL_OS_WINDOWS)
   if (thread->handle == NULL) {
      #ifdef CFL_THREAD_WINRAWAPI
         DWORD threadId;
         thread->param = param;
         thread->handle = CreateThread(NULL, 0, startFunction, thread, 0, &threadId);
      #else
         thread->param = param;
         thread->handle = ( HANDLE ) _beginthreadex( NULL, 0, startFunction, thread, 0, NULL );
      #endif
      return thread->handle != NULL ? CFL_TRUE : CFL_FALSE;
   }
   return CFL_FALSE;
#else
   thread->param = param;
   return pthread_create(&thread->handle, NULL, startFunction, thread) == 0 ? CFL_TRUE : CFL_FALSE;
#endif
}

CFL_BOOL cfl_thread_wait(CFL_THREADP thread) {
#if defined(CFL_OS_WINDOWS)
   if (thread->handle) {
      if (WaitForSingleObject(thread->handle, INFINITE) == WAIT_OBJECT_0) {
         thread->joined = CFL_TRUE;
         return CFL_TRUE;
      } else {
         return CFL_FALSE;
      }
   }
   return CFL_FALSE;
#else
   if (pthread_join(thread->handle, NULL) == 0) {
      thread->joined = CFL_TRUE;
      return CFL_TRUE;
   } else {
      return CFL_FALSE;
   }
#endif
}

CFL_BOOL cfl_thread_waitTimeout(CFL_THREADP thread, CFL_INT32 timeout) {
#if defined(CFL_OS_WINDOWS)
   if (thread->handle) {
      DWORD res = WaitForSingleObject(thread->handle, timeout);
      if (res == WAIT_OBJECT_0 || res == WAIT_TIMEOUT) {
         thread->joined = CFL_TRUE;
         return CFL_TRUE;
      } else {
         return CFL_FALSE;
      }
   }
   return CFL_FALSE;
#else
   struct timespec ts;

   //
   if (clock_gettime(CLOCK_REALTIME, &ts) == 0) {
      ts.tv_sec += (int) (timeout / 1000);
      ts.tv_nsec += (long) ((timeout % 1000) * 1000000);
      if (pthread_timedjoin_np(thread->handle, NULL, &ts) == 0) {
         thread->joined = CFL_TRUE;
         return CFL_TRUE;
      } else {
         return CFL_FALSE;
      }
   }
#endif
}

CFL_BOOL cfl_thread_kill(CFL_THREADP thread) {
   if (thread->status == CFL_THREAD_RUNNING && KILL_THREAD(thread)) {
      thread->status = CFL_THREAD_KILLED;
      return CFL_TRUE;
   }
   return CFL_FALSE;
}

void cfl_thread_signalError(CFL_THREADP thread) {
   thread->status = CFL_THREAD_ERROR;
}

CFL_UINT8 cfl_thread_status(const CFL_THREADP thread) {
   return thread->status;
}
CFL_BOOL cfl_thread_currentIsHandled(void) {
   INIT_THREAD_STORAGE
   return GET_THREAD() != NULL ? CFL_TRUE : CFL_FALSE;
}

CFL_BOOL cfl_thread_sleep(CFL_UINT32 time) {
#if defined(CFL_OS_WINDOWS)
   Sleep((DWORD) time);
#else
   usleep((useconds_t) time * 1000);
#endif
   return CFL_TRUE;
}

void cfl_thread_yield(void) {
#if defined(CFL_OS_WINDOWS)
   YieldProcessor();
#else
   sched_yield();
#endif
}

static CFL_THREAD_VAR_DATA *thread_dataGet(CFL_THREAD_VARIABLEP threadVar) {
   CFL_THREAD_VAR_DATA *varData;
   INIT_DATA_STORAGE(threadVar);
   varData = (CFL_THREAD_VAR_DATA *) GET_DATA(threadVar);
   if (varData == NULL) {
      size_t dataSize = DATA_SIZE(threadVar);
      varData = CFL_MEM_ALLOC(sizeof(CFL_THREAD_VAR_DATA) + dataSize);
      if (varData != NULL) {
         SET_DATA(threadVar, varData);
         if (threadVar->initData != NULL) {
            threadVar->initData(&varData->data[0]);
         } else {
            memset(&varData->data[0], 0, dataSize);
         }
         varData->var = threadVar;
      }
   }
   return varData;
}

void *cfl_thread_varGet(CFL_THREAD_VARIABLEP threadVar) {
   CFL_THREAD_VAR_DATA *varData = thread_dataGet(threadVar);
   return varData != NULL ? &varData->data[0] : NULL;
}

CFL_BOOL cfl_thread_varSet(CFL_THREAD_VARIABLEP threadVar, const void *data) {
   CFL_THREAD_VAR_DATA *varData = thread_dataGet(threadVar);
   if (varData == NULL || varData == data) {
      return CFL_FALSE;
   }

   if (data != NULL) {
      memcpy(&varData->data[0], data, DATA_SIZE(threadVar));
   } else {
      memset(&varData->data[0], 0, DATA_SIZE(threadVar));
   }
   return CFL_TRUE;
}

void *cfl_thread_varGetPointer(CFL_THREAD_VARIABLEP threadVar) {
   return cfl_thread_varGet(threadVar);
}

CFL_BOOL cfl_thread_varSetPointer(CFL_THREAD_VARIABLEP threadVar, void *data) {
   return cfl_thread_varSet(threadVar, data);
}

DEFINE_GET_SET(CFL_BOOL   , Boolean, CFL_FALSE)
DEFINE_GET_SET(CFL_INT8   , Int8   , 0        )
DEFINE_GET_SET(CFL_INT16  , Int16  , 0        )
DEFINE_GET_SET(CFL_INT32  , Int32  , 0        )
DEFINE_GET_SET(CFL_INT64  , Int64  , 0        )
DEFINE_GET_SET(CFL_UINT8  , UInt8  , 0        )
DEFINE_GET_SET(CFL_UINT16 , UInt16 , 0        )
DEFINE_GET_SET(CFL_UINT32 , UInt32 , 0        )
DEFINE_GET_SET(CFL_UINT64 , UInt64 , 0        )
DEFINE_GET_SET(CFL_FLOAT32, Float32, 0        )
DEFINE_GET_SET(CFL_FLOAT64, Float64, 0        )
