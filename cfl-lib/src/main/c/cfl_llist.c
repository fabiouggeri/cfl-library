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

#include "cfl_llist.h"


CFL_LLISTP cfl_llist_new(CFL_UINT32 maxNodeCache) {
   CFL_LLISTP list = (CFL_LLISTP) CFL_MEM_ALLOC(sizeof(CFL_LLIST));
   list->maxNodeCache = maxNodeCache;
   list->nodeCount = 0;
   list->first = NULL;
   list->last = NULL;
   list->head = NULL;
   list->tail = NULL;
   return list;
}

void cfl_llist_free(CFL_LLISTP list) {
   if (list){
      CFL_LNODEP node = list->first;
      while (node) {
         CFL_LNODEP next = node->next;
         CFL_MEM_FREE(node);
         node = next;
      }
      CFL_MEM_FREE(list);
   }
}

void cfl_llist_addLast(CFL_LLISTP list, void *item) {
   CFL_LNODEP node = list->tail;
   /* Exists tail */
   if (node) {
      /* Has node avaiable after tail */
      if (node->next) {
         node = node->next;
         list->tail = node;

      /* Has node avaiable at the beginning */
      } else if (list->first != list->head) {
         node = list->first;
         /* Remove from begin */
         node->next->previous = NULL;
         list->first = node->next;
         /* Append to end */
         node->next = NULL;
         node->previous = list->last;
         list->last->next = node;
         /* Define node as last and tail */
         list->tail = node;
         list->last = node;
      /* New node at the end of chain */
      } else {
         node = (CFL_LNODEP) CFL_MEM_ALLOC(sizeof(CFL_LNODE));
         node->previous = list->tail;
         node->next = NULL;
         list->tail->next = node;
         list->tail = node;
         list->last = node;
         ++(list->nodeCount);
      }
   /* Exists previous node created */
   } else if (list->first) {
      node = list->first;
      list->head = node;
      list->tail = node;
   /* First node in chain */
   } else {
      node = (CFL_LNODEP) CFL_MEM_ALLOC(sizeof(CFL_LNODE));
      node->previous = NULL;
      node->next = NULL;
      list->first = node;
      list->last = node;
      list->head = node;
      list->tail = node;
      ++(list->nodeCount);
   }
   node->data = item;
}

void cfl_llist_addFirst(CFL_LLISTP list, void *item) {
   CFL_LNODEP node = list->head;
   /* Exists head */
   if (node) {
      /* Has node avaiable before head */
      if (node->previous) {
         node = node->previous;
         list->head = node;

      /* Has node avaiable at the end */
      } else if (list->last != list->tail) {
         node = list->last;
         /* Remove from end */
         node->previous->next = NULL;
         list->last = node->previous;
         /* insert in the beginning */
         node->previous = NULL;
         node->next = list->first;
         list->first->previous = node;
         /* Define node as first and head */
         list->head = node;
         list->first = node;
      /* New node at beginning of chain */
      } else {
         node = (CFL_LNODEP) CFL_MEM_ALLOC(sizeof(CFL_LNODE));
         node->next = list->head;
         node->previous = NULL;
         list->head->previous = node;
         list->head = node;
         list->first = node;
         ++(list->nodeCount);
      }
   /* Exists previous node created */
   } else if (list->first) {
      node = list->first;
      list->head = node;
      list->tail = node;
   /* First node in chain */
   } else {
      node = (CFL_LNODEP) CFL_MEM_ALLOC(sizeof(CFL_LNODE));
      node->previous = NULL;
      node->next = NULL;
      list->first = node;
      list->last = node;
      list->head = node;
      list->tail = node;
      ++(list->nodeCount);
   }
   node->data = item;
}

void *cfl_llist_getLast(const CFL_LLISTP list) {
   return list->tail;
}

void *cfl_llist_getFirst(const CFL_LLISTP list) {
   return list->head;

}

static void checkCacheOverflow(CFL_LLISTP list) {
   /* Remove initial nodes */
   while (list->first != list->head && list->nodeCount > list->maxNodeCache) {
      CFL_LNODEP node = list->first;
      if (node->next) {
         node->next->previous = NULL;
      }
      list->first = node->next;
      CFL_MEM_FREE(node);
      --(list->nodeCount);
   }
   /* Has nodes yet */
   if (list->first) {
      /* Remove final nodes */
      while (list->last != list->tail && list->nodeCount > list->maxNodeCache) {
         CFL_LNODEP node = list->last;
         if (node->previous) {
            node->previous->next = NULL;
         }
         list->last = node->previous;
         CFL_MEM_FREE(node);
         --(list->nodeCount);
      }
   } else {
      list->last = NULL;
   }
}

void *cfl_llist_removeLast(CFL_LLISTP list) {
   void *data;
   if (list->tail) {
      data = list->tail->data;
      if (list->tail == list->head) {
         list->head = NULL;
         list->tail = NULL;
      } else {
         list->tail = list->tail->previous;
      }
      checkCacheOverflow(list);
   } else {
      data = NULL;
   }
   return data;
}

void *cfl_llist_removeFirst(CFL_LLISTP list) {
   void *data;
   if (list->head) {
      data = list->head->data;
      if (list->head == list->tail) {
         list->head = NULL;
         list->tail = NULL;
      } else {
         list->head = list->head->next;
      }
      checkCacheOverflow(list);
   } else {
      data = NULL;
   }
   return data;
}

