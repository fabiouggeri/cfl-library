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

#ifndef CFL_THREAD_H_

#define CFL_THREAD_H_

#include "cfl_types.h"

#define CFL_THREAD_CREATED   0x00
#define CFL_THREAD_RUNNING   0x01
#define CFL_THREAD_FINISHED  0x02
#define CFL_THREAD_KILLED    0x03
#define CFL_THREAD_ERROR     0xFF

#define DECLARE_GET_SET(datatype, typename) \
   extern datatype cfl_thread_varGet##typename(CFL_THREAD_VARIABLEP threadVar); \
   extern CFL_BOOL cfl_thread_varSet##typename(CFL_THREAD_VARIABLEP threadVar, datatype data);

#if defined(CFL_OS_WINDOWS)
   #define CFL_THREAD_VAR_EMPTY { 0, TLS_OUT_OF_INDEXES, sizeof(VAR_DATA), NULL, NULL }
   #define CFL_THREAD_VAR_INIT(datatype, varname, initFunc, freeFunc) CFL_THREAD_VARIABLE varname = { 0, TLS_OUT_OF_INDEXES, sizeof(datatype), initFunc, freeFunc }
   #define CFL_THREAD_VAR(datatype, varname) CFL_THREAD_VARIABLE varname = { 0, TLS_OUT_OF_INDEXES, sizeof(datatype), NULL, NULL }
#else
   #define CFL_THREAD_VAR_EMPTY { 0, 0, sizeof(VAR_DATA), NULL, NULL}
   #define CFL_THREAD_VAR_INIT(datatype, varname, initFunc, freeFunc) CFL_THREAD_VARIABLE varname = { 0, 0, sizeof(datatype), initFunc, freeFunc }
   #define CFL_THREAD_VAR(datatype, varname) CFL_THREAD_VARIABLE varname = { 0, 0, sizeof(datatype), NULL, NULL }
#endif

typedef void (* CFL_THREAD_VAR_FUNC)(void *param);

typedef CFL_INT32 (* CFL_THREAD_FUNC)(void *param);

typedef union {
   CFL_BOOL    asBoolean;
   CFL_INT8    asInt8;
   CFL_INT16   asInt16;
   CFL_INT32   asInt32;
   CFL_INT64   asInt64;
   CFL_UINT8   asUInt8;
   CFL_UINT16  asUInt16;
   CFL_UINT32  asUInt32;
   CFL_UINT64  asUInt64;
   CFL_FLOAT32 asFloat32;
   CFL_FLOAT64 asFloat64;
   void       *asPointer;
} VAR_DATA;

typedef struct _CFL_THREAD_VAR_DATA {
   CFL_THREAD_VARIABLEP var;
   CFL_UINT8            data[1];
} CFL_THREAD_VAR_DATA;

struct _CFL_THREAD_VARIABLE {
   #if defined(CFL_OS_WINDOWS)
      volatile LONG     initialized;
      DWORD             storageKey;
   #else
      int               initialized;
      pthread_key_t     storageKey;
   #endif
   size_t               dataSize;
   CFL_THREAD_VAR_FUNC initData;
   CFL_THREAD_VAR_FUNC freeData;
};

struct _CFL_THREAD {
   CFL_THREAD_FUNC   func;
   void              *param;
   CFL_THREAD_HANDLE handle;
   CFL_BOOL          manualAllocation;
   CFL_BOOL          joined;
   CFL_UINT8         status;
};

extern CFL_THREADP cfl_thread_new(CFL_THREAD_FUNC func);
extern void cfl_thread_free(CFL_THREADP thread);
extern CFL_THREADP cfl_thread_getCurrent(void);
extern void * cfl_thread_getData(CFL_THREADP thread);
extern void cfl_thread_setData(CFL_THREADP thread, void *data);
extern CFL_BOOL cfl_thread_start(CFL_THREADP thread, void * param);
extern CFL_BOOL cfl_thread_wait(CFL_THREADP thread);
extern CFL_BOOL cfl_thread_waitTimeout(CFL_THREADP thread, CFL_INT32 timeout);
extern CFL_BOOL cfl_thread_kill(CFL_THREADP thread);
extern CFL_UINT8 cfl_thread_status(CFL_THREADP thread);
extern void cfl_thread_signalError(CFL_THREADP thread);
extern CFL_BOOL cfl_thread_currentIsHandled(void);
extern CFL_BOOL cfl_thread_sleep(CFL_UINT32 time);

extern void *cfl_thread_varGet(CFL_THREAD_VARIABLEP threadVar);
extern CFL_BOOL cfl_thread_varSet(CFL_THREAD_VARIABLEP threadVar, void *data);

DECLARE_GET_SET(void *     , Pointer)
DECLARE_GET_SET(CFL_BOOL   , Boolean)
DECLARE_GET_SET(CFL_INT8   , Int8   )
DECLARE_GET_SET(CFL_INT16  , Int16  )
DECLARE_GET_SET(CFL_INT32  , Int32  )
DECLARE_GET_SET(CFL_INT64  , Int64  )
DECLARE_GET_SET(CFL_UINT8  , UInt8  )
DECLARE_GET_SET(CFL_UINT16 , UInt16 )
DECLARE_GET_SET(CFL_UINT32 , UInt32 )
DECLARE_GET_SET(CFL_UINT64 , UInt64 )
DECLARE_GET_SET(CFL_FLOAT32, Float32 )
DECLARE_GET_SET(CFL_FLOAT64, Float64 )

#endif