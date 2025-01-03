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
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "cfl_sync_queue.h"
#include "cfl_lock.h"

#define DEFINE_GET_PUT(datatype, typename) \
   datatype cfl_sync_queue_get##typename(CFL_SYNC_QUEUEP queue) { \
      CFL_BOOL timesUp; \
      return getItem(queue, 0, &timesUp).as##typename; \
   } \
   datatype cfl_sync_queue_tryGet##typename(CFL_SYNC_QUEUEP queue, CFL_BOOL *took) { \
      return tryGetItem(queue, took).as##typename; \
   } \
   CFL_BOOL cfl_sync_queue_put##typename(CFL_SYNC_QUEUEP queue, datatype data) { \
      CFL_SYNC_QUEUE_ITEM item; \
      item.as##typename = data; \
      return putItem(queue, item, 0); \
   } \
   CFL_BOOL cfl_sync_queue_tryPut##typename(CFL_SYNC_QUEUEP queue, datatype data) { \
      CFL_SYNC_QUEUE_ITEM item; \
      item.as##typename = data; \
      return tryPutItem(queue, item); \
   }

#define DEFINE_GET_PUT_TIMEOUT(datatype, typename) \
   datatype cfl_sync_queue_get##typename##Timeout(CFL_SYNC_QUEUEP queue, CFL_UINT32 timeout, CFL_BOOL *timesUp) { \
      return getItem(queue, timeout, timesUp).as##typename; \
   } \
   CFL_BOOL cfl_sync_queue_put##typename##Timeout(CFL_SYNC_QUEUEP queue, datatype data, CFL_UINT32 timeout) { \
      CFL_SYNC_QUEUE_ITEM item; \
      item.as##typename = data; \
      return putItem(queue, item, timeout); \
   }

#define DEFINE_DRAIN(datatype, typename) \
   datatype cfl_sync_queue_drain##typename(CFL_SYNC_QUEUEP queue, CFL_BOOL *empty) { \
      return queue_drain(queue, empty).as##typename; \
   }

#define IS_EMPTY(c)   ((c)->itemCount == 0)
#define IS_FULL(c)    ((c)->itemCount == (c)->size)
#define NEXT_INDEX(c) (((c)->index + (c)->itemCount) % (c)->size)

CFL_SYNC_QUEUEP cfl_sync_queue_new(CFL_UINT32 size) {
   CFL_SYNC_QUEUEP queue;
   if (size == 0) {
      return NULL;
   }
   queue = (CFL_SYNC_QUEUEP) CFL_MEM_ALLOC(sizeof(CFL_SYNC_QUEUE) + (size * sizeof(CFL_SYNC_QUEUE_ITEM)));
   if (queue) {
      cfl_lock_init(&queue->lock);
      queue->notEmpty = cfl_lock_newConditionVar();
      queue->notFull = cfl_lock_newConditionVar();
      queue->index = 0;
      queue->itemCount = 0;
      queue->size = size;
      queue->canceled = CFL_FALSE;
   }
   return queue;
}

void cfl_sync_queue_free(CFL_SYNC_QUEUEP queue) {
   if (queue) {
      cfl_sync_queue_cancel(queue);
      cfl_lock_freeConditionVar(queue->notEmpty);
      cfl_lock_freeConditionVar(queue->notFull);
      cfl_lock_free(&queue->lock);
      CFL_MEM_FREE(queue);
   }
}

CFL_BOOL cfl_sync_queue_isEmpty(CFL_SYNC_QUEUEP queue) {
   return IS_EMPTY(queue);
}

CFL_BOOL cfl_sync_queue_isFull(CFL_SYNC_QUEUEP queue) {
   return IS_FULL(queue);
}

static CFL_SYNC_QUEUE_ITEM getItem(CFL_SYNC_QUEUEP queue, CFL_UINT32 timeout, CFL_BOOL *timesUp) {
   CFL_SYNC_QUEUE_ITEM data = {0};
   cfl_lock_acquire(&queue->lock);
   while (IS_EMPTY(queue) && ! queue->canceled) {
      if (timeout == 0) {
         cfl_lock_conditionWait(&queue->lock, queue->notEmpty);
      } else {
         switch (cfl_lock_conditionWaitTimeout(&queue->lock, queue->notEmpty, timeout)) {
            case CFL_LOCK_TIMEOUT:
               *timesUp = CFL_TRUE;
            case CFL_LOCK_ERROR:
               cfl_lock_release(&queue->lock);
               return data;
         }
      }
   }
   if (! queue->canceled) {
      data = queue->data[queue->index];
      --queue->itemCount;
      ++queue->index;
      if (queue->index >= queue->size) {
         queue->index = 0;
      }
   }
   cfl_lock_conditionWakeAll(queue->notFull);
   cfl_lock_release(&queue->lock);
   *timesUp = CFL_FALSE;
   return data;
}

static CFL_SYNC_QUEUE_ITEM tryGetItem(CFL_SYNC_QUEUEP queue, CFL_BOOL *took) {
   CFL_SYNC_QUEUE_ITEM data = {0};
   cfl_lock_acquire(&queue->lock);
   if (! IS_EMPTY(queue) && ! queue->canceled) {
      data = queue->data[queue->index];
      --queue->itemCount;
      ++queue->index;
      if (queue->index >= queue->size) {
         queue->index = 0;
      }
      *took = CFL_TRUE;
      cfl_lock_conditionWakeAll(queue->notFull);
      cfl_lock_release(&queue->lock);
   } else {
      *took = CFL_FALSE;
      cfl_lock_release(&queue->lock);
   }
   return data;
}

static CFL_BOOL putItem(CFL_SYNC_QUEUEP queue, CFL_SYNC_QUEUE_ITEM item, CFL_UINT32 timeout) {
   cfl_lock_acquire(&queue->lock);
   while (IS_FULL(queue) && ! queue->canceled) {
      if (timeout == 0) {
         cfl_lock_conditionWait(&queue->lock, queue->notFull);
      } else if (cfl_lock_conditionWaitTimeout(&queue->lock, queue->notFull, timeout) != CFL_LOCK_SUCCESS) {
         cfl_lock_release(&queue->lock);
         return CFL_FALSE;
      }
   }
   if (! queue->canceled) {
      queue->data[NEXT_INDEX(queue)] = item;
      ++queue->itemCount;
   }
   cfl_lock_conditionWakeAll(queue->notEmpty);
   cfl_lock_release(&queue->lock);
   return CFL_TRUE;
}

static CFL_BOOL tryPutItem(CFL_SYNC_QUEUEP queue, CFL_SYNC_QUEUE_ITEM item) {
   cfl_lock_acquire(&queue->lock);
   if (! IS_FULL(queue) && ! queue->canceled) {
      queue->data[NEXT_INDEX(queue)] = item;
      ++queue->itemCount;
      cfl_lock_conditionWakeAll(queue->notEmpty);
      cfl_lock_release(&queue->lock);
      return CFL_TRUE;
   } else {
      cfl_lock_release(&queue->lock);
      return CFL_FALSE;
   }
}

static CFL_UINT32 waitNotEmpty(CFL_SYNC_QUEUEP queue, CFL_UINT32 timeout, CFL_BOOL *timesUp) {
   CFL_UINT32 itemsCount;
   cfl_lock_acquire(&queue->lock);
   while (IS_EMPTY(queue) && ! queue->canceled) {
      if (timeout == 0) {
         cfl_lock_conditionWait(&queue->lock, queue->notEmpty);
      } else {
         switch (cfl_lock_conditionWaitTimeout(&queue->lock, queue->notEmpty, timeout)) {
            case CFL_LOCK_TIMEOUT:
               itemsCount = queue->itemCount;
               cfl_lock_release(&queue->lock);
               *timesUp = CFL_TRUE;
               return itemsCount;
            case CFL_LOCK_ERROR:
               cfl_lock_release(&queue->lock);
               *timesUp = CFL_FALSE;
               return 0;
         }
      }
   }
   itemsCount = queue->itemCount;
   cfl_lock_release(&queue->lock);
   *timesUp = CFL_FALSE;
   return itemsCount;
}

CFL_UINT32 cfl_sync_queue_waitNotEmptyTimeout(CFL_SYNC_QUEUEP queue, CFL_UINT32 timeout, CFL_BOOL *timesUp) {
   return waitNotEmpty(queue, timeout, timesUp);
}

CFL_UINT32 cfl_sync_queue_waitNotEmpty(CFL_SYNC_QUEUEP queue) {
   CFL_BOOL timesUp;
   return waitNotEmpty(queue, 0, &timesUp);
}

static CFL_UINT32 waitEmpty(CFL_SYNC_QUEUEP queue, CFL_UINT32 timeout, CFL_BOOL *timesUp) {
   CFL_UINT32 itemCount;
   cfl_lock_acquire(&queue->lock);
   while (! IS_EMPTY(queue) && ! queue->canceled) {
      if (timeout == 0) {
         cfl_lock_conditionWait(&queue->lock, queue->notFull);
      } else {
         clock_t elapsed; 
         clock_t start = clock();
         switch (cfl_lock_conditionWaitTimeout(&queue->lock, queue->notFull, timeout)) {
            case CFL_LOCK_TIMEOUT:
               itemCount = queue->itemCount;
               cfl_lock_release(&queue->lock);
               *timesUp = CFL_TRUE;
               return itemCount;
            case CFL_LOCK_ERROR:
               itemCount = queue->itemCount;
               cfl_lock_release(&queue->lock);
               *timesUp = CFL_FALSE;
               return itemCount;
         }
         elapsed = clock() - start;
         if (elapsed >= (clock_t)timeout) {
            itemCount = queue->itemCount;
            cfl_lock_release(&queue->lock);
            *timesUp = CFL_TRUE;
            return itemCount;
         } else {
            timeout -= (CFL_UINT32)elapsed;
         }
      }
   }
   itemCount = queue->itemCount;
   cfl_lock_release(&queue->lock);
   *timesUp = CFL_FALSE;
   return itemCount;
}

CFL_UINT32 cfl_sync_queue_waitEmptyTimeout(CFL_SYNC_QUEUEP queue, CFL_UINT32 timeout, CFL_BOOL *timesUp) {
   return waitEmpty(queue, timeout, timesUp);
}

CFL_UINT32 cfl_sync_queue_waitEmpty(CFL_SYNC_QUEUEP queue) {
   CFL_BOOL timesUp;
   return waitEmpty(queue, 0, &timesUp);
}

void * cfl_sync_queue_get(CFL_SYNC_QUEUEP queue) {
   CFL_BOOL timesUp;
   return getItem(queue, 0, &timesUp).asPointer;
}

void * cfl_sync_queue_tryGet(CFL_SYNC_QUEUEP queue, CFL_BOOL *took) {
   return tryGetItem(queue, took).asPointer;
}

CFL_BOOL cfl_sync_queue_put(CFL_SYNC_QUEUEP queue, void *data) {
   CFL_SYNC_QUEUE_ITEM item;
   item.asPointer = data;
   return putItem(queue, item, 0);
}

CFL_BOOL cfl_sync_queue_tryPut(CFL_SYNC_QUEUEP queue, void *data) {
   CFL_SYNC_QUEUE_ITEM item;
   item.asPointer = data;
   return tryPutItem(queue, item);
}

void cfl_sync_queue_cancel(CFL_SYNC_QUEUEP queue) {
   if (! queue->canceled) {
      queue->canceled = CFL_TRUE;
      cfl_lock_conditionWakeAll(queue->notEmpty);
      cfl_lock_conditionWakeAll(queue->notFull);
   }
}

static CFL_SYNC_QUEUE_ITEM queue_drain(CFL_SYNC_QUEUEP queue, CFL_BOOL *empty) {
   CFL_SYNC_QUEUE_ITEM data = {0};
   cfl_lock_acquire(&queue->lock);
   if (! IS_EMPTY(queue)) {
      *empty = CFL_FALSE;
      data = queue->data[queue->index];
      --queue->itemCount;
      ++queue->index;
      if (queue->index >= queue->size) {
         queue->index = 0;
      }
   } else {
      *empty = CFL_TRUE;
   }
   cfl_lock_release(&queue->lock);
   return data;
}

CFL_BOOL cfl_sync_queue_canceled(CFL_SYNC_QUEUEP queue) {
   return queue->canceled;
}

DEFINE_GET_PUT(CFL_BOOL  , Boolean)
DEFINE_GET_PUT(CFL_INT8  , Int8   )
DEFINE_GET_PUT(CFL_INT16 , Int16  )
DEFINE_GET_PUT(CFL_INT32 , Int32  )
DEFINE_GET_PUT(CFL_INT64 , Int64  )
DEFINE_GET_PUT(CFL_UINT8 , UInt8  )
DEFINE_GET_PUT(CFL_UINT16, UInt16 )
DEFINE_GET_PUT(CFL_UINT32, UInt32 )
DEFINE_GET_PUT(CFL_UINT64, UInt64 )

void * cfl_sync_queue_getTimeout(CFL_SYNC_QUEUEP queue, CFL_UINT32 timeout, CFL_BOOL *timesUp) {
   return getItem(queue, timeout, timesUp).asPointer;
}

CFL_BOOL cfl_sync_queue_putTimeout(CFL_SYNC_QUEUEP queue, void *data, CFL_UINT32 timeout) {
   CFL_SYNC_QUEUE_ITEM item;
   item.asPointer = data;
   return putItem(queue, item, timeout);
}

DEFINE_GET_PUT_TIMEOUT(CFL_BOOL  , Boolean)
DEFINE_GET_PUT_TIMEOUT(CFL_INT8  , Int8   )
DEFINE_GET_PUT_TIMEOUT(CFL_INT16 , Int16  )
DEFINE_GET_PUT_TIMEOUT(CFL_INT32 , Int32  )
DEFINE_GET_PUT_TIMEOUT(CFL_INT64 , Int64  )
DEFINE_GET_PUT_TIMEOUT(CFL_UINT8 , UInt8  )
DEFINE_GET_PUT_TIMEOUT(CFL_UINT16, UInt16 )
DEFINE_GET_PUT_TIMEOUT(CFL_UINT32, UInt32 )
DEFINE_GET_PUT_TIMEOUT(CFL_UINT64, UInt64 )


void *cfl_sync_queue_drain(CFL_SYNC_QUEUEP queue, CFL_BOOL *empty) {
   return queue_drain(queue, empty).asPointer;
}

DEFINE_DRAIN(CFL_BOOL  , Boolean)
DEFINE_DRAIN(CFL_INT8  , Int8   )
DEFINE_DRAIN(CFL_INT16 , Int16  )
DEFINE_DRAIN(CFL_INT32 , Int32  )
DEFINE_DRAIN(CFL_INT64 , Int64  )
DEFINE_DRAIN(CFL_UINT8 , UInt8  )
DEFINE_DRAIN(CFL_UINT16, UInt16 )
DEFINE_DRAIN(CFL_UINT32, UInt32 )
DEFINE_DRAIN(CFL_UINT64, UInt64 )
