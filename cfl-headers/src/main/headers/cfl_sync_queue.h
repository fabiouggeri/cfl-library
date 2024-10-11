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

#ifndef _CFL_SYNC_QUEUE_H_

#define _CFL_SYNC_QUEUE_H_

#include "cfl_types.h"
#include "cfl_lock.h"

#ifdef __cplusplus
extern "C" {
#endif

#define DECLARE_GET_PUT(datatype, typename) \
   extern datatype cfl_sync_queue_get##typename(CFL_SYNC_QUEUEP queue); \
   extern datatype cfl_sync_queue_tryGet##typename(CFL_SYNC_QUEUEP queue, CFL_BOOL *took); \
   extern CFL_BOOL cfl_sync_queue_put##typename(CFL_SYNC_QUEUEP queue, datatype data); \
   extern CFL_BOOL cfl_sync_queue_tryPut##typename(CFL_SYNC_QUEUEP queue, datatype data)

#define DECLARE_GET_PUT_TIMEOUT(datatype, typename) \
   extern datatype cfl_sync_queue_get##typename##Timeout(CFL_SYNC_QUEUEP queue, CFL_UINT32 timeout, CFL_BOOL *timesUp); \
   extern CFL_BOOL cfl_sync_queue_put##typename##Timeout(CFL_SYNC_QUEUEP queue, datatype data, CFL_UINT32 timeout)

#define DECLARE_DRAIN(datatype, typename) \
   extern datatype cfl_sync_queue_drain##typename(CFL_SYNC_QUEUEP queue, CFL_BOOL *empty)

typedef union _CFL_SYNC_QUEUE_ITEM {
   void      *asPointer;
   CFL_BOOL   asBoolean;
   CFL_INT8   asInt8;
   CFL_INT16  asInt16;
   CFL_INT32  asInt32;
   CFL_INT64  asInt64;
   CFL_UINT8  asUInt8;
   CFL_UINT16 asUInt16;
   CFL_UINT32 asUInt32;
   CFL_UINT64 asUInt64;
} CFL_SYNC_QUEUE_ITEM;

struct _CFL_SYNC_QUEUE {
   CFL_LOCK                lock;
   CFL_CONDITION_VARIABLEP notEmpty;
   CFL_CONDITION_VARIABLEP notFull;
	CFL_UINT32              size;
	CFL_UINT32              index;
	CFL_UINT32              itemCount;
   CFL_BOOL                canceled;
	CFL_SYNC_QUEUE_ITEM     data[1];
};

extern CFL_SYNC_QUEUEP cfl_sync_queue_new(CFL_UINT32 size);
extern void cfl_sync_queue_free(CFL_SYNC_QUEUEP queue);
extern CFL_BOOL cfl_sync_queue_isEmpty(CFL_SYNC_QUEUEP queue);
extern CFL_BOOL cfl_sync_queue_isFull(CFL_SYNC_QUEUEP queue);
extern void * cfl_sync_queue_get(CFL_SYNC_QUEUEP queue);
extern void * cfl_sync_queue_tryGet(CFL_SYNC_QUEUEP queue, CFL_BOOL *took);
extern void * cfl_sync_queue_getTimeout(CFL_SYNC_QUEUEP queue, CFL_UINT32 timeout, CFL_BOOL *timesUp);
extern CFL_BOOL cfl_sync_queue_put(CFL_SYNC_QUEUEP queue, void *data);
extern CFL_BOOL cfl_sync_queue_tryPut(CFL_SYNC_QUEUEP queue, void *data);
extern CFL_BOOL cfl_sync_queue_putTimeout(CFL_SYNC_QUEUEP queue, void *data, CFL_UINT32 timeout);
extern void cfl_sync_queue_cancel(CFL_SYNC_QUEUEP queue);
extern CFL_BOOL cfl_sync_queue_canceled(CFL_SYNC_QUEUEP queue);
extern void *cfl_sync_queue_drain(CFL_SYNC_QUEUEP queue, CFL_BOOL *empty);
extern CFL_UINT32 cfl_sync_queue_waitNotEmptyTimeout(CFL_SYNC_QUEUEP queue, CFL_UINT32 timeout, CFL_BOOL *timesUp);
extern CFL_UINT32 cfl_sync_queue_waitNotEmpty(CFL_SYNC_QUEUEP queue);
extern CFL_UINT32 cfl_sync_queue_waitEmptyTimeout(CFL_SYNC_QUEUEP queue, CFL_UINT32 timeout, CFL_BOOL *timesUp);
extern CFL_UINT32 cfl_sync_queue_waitEmpty(CFL_SYNC_QUEUEP queue);

DECLARE_GET_PUT(CFL_BOOL    , Boolean);
DECLARE_GET_PUT(CFL_INT8    , Int8   );
DECLARE_GET_PUT(CFL_INT16   , Int16  );
DECLARE_GET_PUT(CFL_INT32   , Int32  );
DECLARE_GET_PUT(CFL_INT64   , Int64  );
DECLARE_GET_PUT(CFL_UINT8   , UInt8  );
DECLARE_GET_PUT(CFL_UINT16  , UInt16 );
DECLARE_GET_PUT(CFL_UINT32  , UInt32 );
DECLARE_GET_PUT(CFL_UINT64  , UInt64 );

DECLARE_GET_PUT_TIMEOUT(CFL_BOOL    , Boolean);
DECLARE_GET_PUT_TIMEOUT(CFL_INT8    , Int8   );
DECLARE_GET_PUT_TIMEOUT(CFL_INT16   , Int16  );
DECLARE_GET_PUT_TIMEOUT(CFL_INT32   , Int32  );
DECLARE_GET_PUT_TIMEOUT(CFL_INT64   , Int64  );
DECLARE_GET_PUT_TIMEOUT(CFL_UINT8   , UInt8  );
DECLARE_GET_PUT_TIMEOUT(CFL_UINT16  , UInt16 );
DECLARE_GET_PUT_TIMEOUT(CFL_UINT32  , UInt32 );
DECLARE_GET_PUT_TIMEOUT(CFL_UINT64  , UInt64 );

DECLARE_DRAIN(CFL_BOOL    , Boolean);
DECLARE_DRAIN(CFL_INT8    , Int8   );
DECLARE_DRAIN(CFL_INT16   , Int16  );
DECLARE_DRAIN(CFL_INT32   , Int32  );
DECLARE_DRAIN(CFL_INT64   , Int64  );
DECLARE_DRAIN(CFL_UINT8   , UInt8  );
DECLARE_DRAIN(CFL_UINT16  , UInt16 );
DECLARE_DRAIN(CFL_UINT32  , UInt32 );
DECLARE_DRAIN(CFL_UINT64  , UInt64 );

#ifdef __cplusplus
}
#endif

#endif
