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

#ifndef CFL_LLIST_H_

#define CFL_LLIST_H_

#include "cfl_types.h"

typedef struct _CFL_LNODE {
   void *data;
   struct _CFL_LNODE *previous;
   struct _CFL_LNODE *next;
} CFL_LNODE, *CFL_LNODEP;

struct _CFL_LLIST {
   CFL_LNODEP first;
   CFL_LNODEP last;
   CFL_LNODEP head;
   CFL_LNODEP tail;
   CFL_UINT32 maxNodeCache;
   CFL_UINT32 nodeCount;
};

extern CFL_LLISTP cfl_llist_new(CFL_UINT32 maxNodeCache);
extern void cfl_llist_free(CFL_LLISTP list);
extern void cfl_llist_addLast(CFL_LLISTP list, void *item);
extern void cfl_llist_addFirst(CFL_LLISTP list, void *item);
extern void *cfl_llist_getLast(CFL_LLISTP list);
extern void *cfl_llist_getFirst(CFL_LLISTP list);
extern void *cfl_llist_removeLast(CFL_LLISTP list);
extern void *cfl_llist_removeFirst(CFL_LLISTP list);

#endif

