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
#include <string.h>
#include "cfl_array.h"
#include "cfl_iterator.h"

typedef struct _ARRAY_ITERATOR {
   CFL_ARRAYP array;
   CFL_UINT32 index;
} ARRAY_ITERATOR, *ARRAY_ITERATORP;

static CFL_BOOL iteratorHasNext(CFL_ITERATORP it);
static void * iteratorNext(CFL_ITERATORP it);
static void * iteratorValue(CFL_ITERATORP it);
static void iteratorRemove(CFL_ITERATORP it);
static void iteratorFirst(CFL_ITERATORP it);
static CFL_BOOL iteratorHasPrevious(CFL_ITERATORP it);
static void * iteratorPrevious(CFL_ITERATORP it);
static void iteratorLast(CFL_ITERATORP it);

static const CFL_ITERATOR_CLASS s_arrayIteratorClass = {
   iteratorHasNext,
   iteratorNext,
   iteratorValue,
   iteratorRemove,
   NULL,
   iteratorFirst,
   iteratorHasPrevious,
   iteratorPrevious,
   iteratorLast,
   NULL
};

void cfl_array_init(CFL_ARRAYP array, CFL_UINT32 ulCapacity, CFL_UINT32 ulItemSize) {
   array->ulItemSize = ulItemSize;
   array->ulLength = 0;
   array->ulCapacity = ulCapacity;
   array->allocated = CFL_FALSE;
   if (ulCapacity > 0) {
      array->items = (CFL_UINT8 *)malloc(ulCapacity * ulItemSize);
   } else {
      array->items = NULL;
   }
}

CFL_ARRAYP cfl_array_new(CFL_UINT32 ulCapacity, CFL_UINT32 ulItemSize) {
   CFL_ARRAYP array;
   if (ulItemSize == 0) {
      return NULL;
   }
   array = (CFL_ARRAYP) malloc(sizeof(CFL_ARRAY));
   if (array != NULL) {
      cfl_array_init(array, ulCapacity, ulItemSize);
      array->allocated = CFL_TRUE;
   }
   return array;
}

CFL_ARRAYP cfl_array_newLen(CFL_UINT32 ulLen, CFL_UINT32 ulItemSize) {
   CFL_ARRAYP array;

   if (ulItemSize > 0) {
      array = (CFL_ARRAYP) malloc(sizeof(CFL_ARRAY));
      cfl_array_init(array, ulLen, ulItemSize);
      array->allocated = CFL_TRUE;
      array->ulLength = ulLen;
   } else {
      array = NULL;
   }
   return array;
}

void cfl_array_free(CFL_ARRAYP array) {
   if (array != NULL) {
      if (array->items != NULL) {
         free(array->items);
      }
      if (array->allocated) {
         free(array);
      }
   }
}

void *cfl_array_add(CFL_ARRAYP array) {
   void *item;
   if (array->ulLength >= array->ulCapacity) {
      if ( array->items != NULL ) {
         array->ulCapacity = ( array->ulCapacity >> 1 ) + 1 + array->ulLength;
         array->items = (CFL_UINT8 *) realloc(array->items, array->ulCapacity * array->ulItemSize);
      } else {
         array->ulCapacity = 12;
         array->items = (CFL_UINT8 *) malloc(array->ulCapacity * array->ulItemSize);
      }
   }
   item = (void *) &array->items[array->ulLength * array->ulItemSize];
   ++(array->ulLength);
   return item;
}

void *cfl_array_insert(CFL_ARRAYP array, CFL_UINT32 ulIndex) {
   CFL_UINT32 ulNewLen = ulIndex < array->ulLength ? array->ulLength : ulIndex;
   if (ulNewLen >= array->ulCapacity) {
      if ( array->items != NULL ) {
         array->ulCapacity = ( array->ulCapacity >> 1 ) + 1 + ulNewLen;
         array->items = (CFL_UINT8 *) realloc(array->items, array->ulCapacity * array->ulItemSize);
      } else {
         array->ulCapacity = ( ulNewLen >> 1 ) + 1 + ulNewLen;
         array->items = (CFL_UINT8 *) malloc(array->ulCapacity * array->ulItemSize);
      }
   }
   if (ulIndex < array->ulLength) {
      memmove(&array->items[(ulIndex + 1) * array->ulItemSize],
              &array->items[ulIndex * array->ulItemSize],
              (array->ulLength - ulIndex) * array->ulItemSize);
      ++(array->ulLength);
   } else {
      array->ulLength = ulIndex + 1;
   }
   return (void *) &array->items[ulIndex * array->ulItemSize];
}

void cfl_array_del(CFL_ARRAYP array, CFL_UINT32 ulIndex) {
   if (ulIndex < array->ulLength) {
      --(array->ulLength);
      if (array->ulLength > 0 && ulIndex < array->ulLength) {
         memmove(&array->items[ulIndex * array->ulItemSize],
                 &array->items[(ulIndex + 1) * array->ulItemSize],
                 (array->ulLength - ulIndex - 1) * array->ulItemSize);
      }
   }
}

void *cfl_array_get(CFL_ARRAYP array, CFL_UINT32 ulIndex) {
   if (ulIndex < array->ulLength) {
      return (void *) &array->items[ ulIndex * array->ulItemSize];
   }
   return NULL;
}

CFL_UINT32 cfl_array_getItemPos(CFL_ARRAYP array, void *item) {
   if (array->ulLength > 0 &&
       (CFL_UINT8 *) item >= array->items &&
       (CFL_UINT8 *)item <= &array->items[(array->ulLength - 1) * array->ulItemSize]) {
      return (CFL_UINT32) (((CFL_UINT8 *) item - array->items) / array->ulItemSize);
   }
   return array->ulLength;
}

void cfl_array_set(CFL_ARRAYP array, CFL_UINT32 ulIndex, void *item) {
   if (ulIndex < array->ulLength) {
      if (item != NULL) {
         memcpy(&array->items[ulIndex * array->ulItemSize], item, array->ulItemSize);
      } else {
         memset(&array->items[ulIndex * array->ulItemSize], 0, array->ulItemSize);
      }
   }
}

void cfl_array_clear(CFL_ARRAYP array) {
   if (array != NULL) {
      array->ulLength = 0;
   }
}

CFL_UINT32 cfl_array_length(CFL_ARRAYP array) {
   if (array != NULL) {
      return array->ulLength;
   }
   return 0;
}

void cfl_array_setLength(CFL_ARRAYP array, CFL_UINT32 newLen) {
   if (array->items == NULL) {
      array->ulCapacity = ( newLen >> 1 ) + 1 + newLen;
      array->items = (CFL_UINT8 *) malloc(array->ulCapacity * array->ulItemSize);
   } else if (newLen < array->ulLength) {
      array->ulLength = newLen;
   } else if (newLen > array->ulLength) {
      if (newLen > array->ulCapacity) {
         array->ulCapacity = ( newLen >> 1 ) + 1 + newLen;
         array->items = (void *) realloc(array->items, array->ulCapacity * array->ulItemSize);
      }
      memset(&array->items[array->ulLength * array->ulItemSize], 0 , (newLen - array->ulLength) * array->ulItemSize);
      array->ulLength = newLen;
   }
}

CFL_ARRAYP cfl_array_clone(CFL_ARRAYP other) {
   CFL_ARRAYP clone = cfl_array_newLen(other->ulLength, other->ulItemSize);
   memcpy(clone->items, other->items, other->ulLength * other->ulItemSize);
   return clone;
}

static CFL_BOOL iteratorHasNext(CFL_ITERATORP it) {
   ARRAY_ITERATORP data = (ARRAY_ITERATORP) cfl_iterator_data(it);
   return cfl_array_length(data->array) - data->index > 0;
}

static void * iteratorNext(CFL_ITERATORP it) {
   ARRAY_ITERATORP data = (ARRAY_ITERATORP) cfl_iterator_data(it);
   if (data->index < cfl_array_length(data->array)) {
      return cfl_array_get(data->array, (data->index)++);
   } else {
      return NULL;
   }
}

static void * iteratorValue(CFL_ITERATORP it) {
   ARRAY_ITERATORP data = (ARRAY_ITERATORP) cfl_iterator_data(it);
   if (data->index > 0) {
      return cfl_array_get(data->array, data->index - 1);
   } else {
      return NULL;
   }
}

static void iteratorRemove(CFL_ITERATORP it) {
   ARRAY_ITERATORP data = (ARRAY_ITERATORP) cfl_iterator_data(it);
   if (data->index > 0) {
      cfl_array_del(data->array, data->index - 1);
   }
}

static void iteratorFirst(CFL_ITERATORP it) {
   ARRAY_ITERATORP data = (ARRAY_ITERATORP) cfl_iterator_data(it);
   data->index = cfl_array_length(data->array) > 0 ? 1 : 0;
}

static CFL_BOOL iteratorHasPrevious(CFL_ITERATORP it) {
   ARRAY_ITERATORP data = (ARRAY_ITERATORP) cfl_iterator_data(it);
   return data->index > 1;
}

static void * iteratorPrevious(CFL_ITERATORP it) {
   ARRAY_ITERATORP data = (ARRAY_ITERATORP) cfl_iterator_data(it);
   if (data->index > 0) {
      --(data->index);
   }
   if (data->index > 0) {
      return cfl_array_get(data->array, data->index - 1);
   } else {
      return NULL;
   }
}

static void iteratorLast(CFL_ITERATORP it) {
   ARRAY_ITERATORP data = (ARRAY_ITERATORP) cfl_iterator_data(it);
   data->index = cfl_array_length(data->array);
}

CFL_ITERATORP cfl_array_iterator(CFL_ARRAYP array) {
   CFL_ITERATORP it = cfl_iterator_new(sizeof(ARRAY_ITERATOR));
   ARRAY_ITERATORP data = (ARRAY_ITERATORP) cfl_iterator_data(it);
   it->itClass = (CFL_ITERATOR_CLASS *) &s_arrayIteratorClass;
   data->array = array;
   data->index = 0;
   return it;
}
