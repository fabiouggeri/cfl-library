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

CFL_LISTP cfl_list_new(CFL_UINT32 ulCapacity) {
   CFL_LISTP list;

   list = (CFL_LISTP) malloc(sizeof(CFL_LIST));
   list->ulLength = 0;
   list->ulCapacity = ulCapacity;
   list->items = (void *)malloc(ulCapacity * sizeof(void *));
   return list;
}

CFL_LISTP cfl_list_newLen(CFL_UINT32 ulLen) {
   CFL_LISTP list;

   list = (CFL_LISTP) malloc(sizeof(CFL_LIST));
   list->ulLength = ulLen;
   list->ulCapacity = ulLen;
   list->items = (void *)malloc(ulLen * sizeof(void *));
   return list;
}

void cfl_list_free(CFL_LISTP list) {
   if (list != NULL) {
      free(list->items);
      free(list);
   }
}

void cfl_list_add(CFL_LISTP list, void *item) {
   if (list->ulLength >= list->ulCapacity) {
      if ( list->ulCapacity > 0 ) {
         list->ulCapacity = ( list->ulCapacity >> 1 ) + 1 + list->ulLength;
         list->items = (void *) realloc(list->items, list->ulCapacity * sizeof(void *));
      } else {
         list->ulCapacity = 12;
         list->items = (void *) malloc(list->ulCapacity * sizeof(void *));
      }
   }
   list->items[list->ulLength] = item;
   ++(list->ulLength);
}

void cfl_list_del(CFL_LISTP list, CFL_UINT32 ulIndex) {
   if (ulIndex >= 0 && ulIndex < list->ulLength) {
      CFL_UINT32 i;
      --(list->ulLength);
      for (i = ulIndex; i < list->ulLength; i++) {
         list->items[i] = list->items[i + 1];
      }
      list->items[list->ulLength] = NULL;
   }
}

void *cfl_list_remove(CFL_LISTP list, CFL_UINT32 ulIndex) {
   if (ulIndex >= 0 && ulIndex < list->ulLength) {
      CFL_UINT32 i;
      void *removed = list->items[ulIndex];
      --(list->ulLength);
      for (i = ulIndex; i < list->ulLength; i++) {
         list->items[i] = list->items[i + 1];
      }
      list->items[list->ulLength] = NULL;
      return removed;
   }
   return NULL;
}

void *cfl_list_removeLast(CFL_LISTP list) {
   return cfl_list_remove(list, list->ulLength - 1);
}

void *cfl_list_get(CFL_LISTP list, CFL_UINT32 ulIndex) {
   if (ulIndex >= 0 && ulIndex < list->ulLength) {
      return list->items[ ulIndex ];
   }
   return NULL;
}

void cfl_list_set(CFL_LISTP list, CFL_UINT32 ulIndex, void *item) {
   if (ulIndex >= 0 && ulIndex < list->ulLength) {
      list->items[ ulIndex ] = item;
   }
}

void cfl_list_clear(CFL_LISTP list) {
   if (list != NULL) {
      list->ulLength = 0;
   }
}

CFL_UINT32 cfl_list_length(CFL_LISTP list) {
   if (list != NULL) {
      return list->ulLength;
   }
   return 0;
}

void cfl_list_setLength(CFL_LISTP list, CFL_UINT32 newLen) {
   if (newLen < list->ulLength) {
      list->ulLength = newLen;
   } else if (newLen > list->ulLength) {
      if (newLen > list->ulCapacity) {
         list->ulCapacity = ( newLen >> 1 ) + 1 + newLen;
         list->items = (void *) realloc(list->items, list->ulCapacity * sizeof(void *));
      }
      memset(&list->items[list->ulLength] ,0 , newLen - list->ulLength);
      list->ulLength = newLen;
   }
}

CFL_LISTP cfl_list_clone(CFL_LISTP other) {
   CFL_LISTP clone = cfl_list_newLen(other->ulLength);
   memcpy(clone->items, other->items, other->ulLength * sizeof(void *));
   return clone;
}
