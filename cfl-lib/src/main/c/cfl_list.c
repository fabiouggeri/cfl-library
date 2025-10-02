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
#include "cfl_list.h"
#include "cfl_mem.h"

void cfl_list_init(CFL_LISTP list, CFL_UINT32 capacity) {
   list->length = 0;
   list->capacity = capacity;
   list->allocated = CFL_FALSE;
   if (capacity > 0) {
      list->items = (void *)CFL_MEM_ALLOC(capacity * sizeof(void *));
   } else {
      list->items = NULL;
   }
}

CFL_LISTP cfl_list_new(CFL_UINT32 capacity) {
   CFL_LISTP list = (CFL_LISTP) CFL_MEM_ALLOC(sizeof(CFL_LIST));
   if (list == NULL) {
      return NULL;
   }
   cfl_list_init(list, capacity);
   list->allocated = CFL_TRUE;
   return list;
}

CFL_LISTP cfl_list_newLen(CFL_UINT32 len) {
   CFL_LISTP list = (CFL_LISTP) CFL_MEM_ALLOC(sizeof(CFL_LIST));
   if (list == NULL) {
      return NULL;
   }
   cfl_list_init(list, len);
   list->allocated = CFL_TRUE;
   list->length = len;
   memset(list->items , 0, len * sizeof(void *));
   return list;
}

void cfl_list_free(CFL_LISTP list) {
   if (list != NULL) {
      if (list->items) {
         CFL_MEM_FREE(list->items);
         list->items = NULL;
      }
      if (list->allocated) {
         CFL_MEM_FREE(list);
      }
   }
}

void cfl_list_add(CFL_LISTP list, void *item) {
   if (list->length >= list->capacity) {
      if ( list->capacity > 0 ) {
         list->capacity = ( list->capacity >> 1 ) + 1 + list->length;
         list->items = (void *) CFL_MEM_REALLOC(list->items, list->capacity * sizeof(void *));
      } else {
         list->capacity = 12;
         list->items = (void *) CFL_MEM_ALLOC(list->capacity * sizeof(void *));
      }
   }
   list->items[list->length] = item;
   ++(list->length);
}

void cfl_list_del(CFL_LISTP list, CFL_UINT32 ulIndex) {
   if (ulIndex < list->length) {
      CFL_UINT32 i;
      --(list->length);
      for (i = ulIndex; i < list->length; i++) {
         list->items[i] = list->items[i + 1];
      }
      list->items[list->length] = NULL;
   }
}

void cfl_list_delItem(CFL_LISTP list, const void *item) {
   CFL_UINT32 i;
   for (i = 0; i < list->length; i++) {
      if (list->items[i] == item) {
         list->length--;
         memmove(&list->items[i], &list->items[i + 1], (list->length - i) * sizeof(void *));
         list->items[list->length] = NULL;
         return;
      }
   }
}

void *cfl_list_remove(CFL_LISTP list, CFL_UINT32 ulIndex) {
   if (ulIndex < list->length) {
      CFL_UINT32 i;
      void *removed = list->items[ulIndex];
      --(list->length);
      for (i = ulIndex; i < list->length; i++) {
         list->items[i] = list->items[i + 1];
      }
      list->items[list->length] = NULL;
      return removed;
   }
   return NULL;
}

void *cfl_list_removeLast(CFL_LISTP list) {
   return cfl_list_remove(list, list->length - 1);
}

void *cfl_list_get(const CFL_LISTP list, CFL_UINT32 ulIndex) {
   if (ulIndex < list->length) {
      return list->items[ ulIndex ];
   }
   return NULL;
}

void cfl_list_set(CFL_LISTP list, CFL_UINT32 ulIndex, void *item) {
   if (ulIndex < list->length) {
      list->items[ ulIndex ] = item;
   }
}

void cfl_list_clear(CFL_LISTP list) {
   if (list != NULL) {
      list->length = 0;
   }
}

CFL_UINT32 cfl_list_length(const CFL_LISTP list) {
   if (list != NULL) {
      return list->length;
   }
   return 0;
}

void cfl_list_setLength(CFL_LISTP list, CFL_UINT32 newLen) {
   if (newLen < list->length) {
      list->length = newLen;
   } else if (newLen > list->length) {
      if (newLen > list->capacity) {
         list->capacity = ( newLen >> 1 ) + 1 + newLen;
         list->items = (void *) CFL_MEM_REALLOC(list->items, list->capacity * sizeof(void *));
      }
      memset(&list->items[list->length] ,0 , (newLen - list->length) * sizeof(void *));
      list->length = newLen;
   }
}

CFL_LISTP cfl_list_clone(const CFL_LISTP other) {
   CFL_LISTP clone = cfl_list_newLen(other->length);
   memcpy(clone->items, other->items, other->length * sizeof(void *));
   return clone;
}
