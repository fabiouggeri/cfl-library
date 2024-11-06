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

#ifndef CFL_LIST_H_

#define CFL_LIST_H_

#include "cfl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _CFL_LIST {
   void       **items;
   CFL_UINT32 length;
   CFL_UINT32 capacity;
   CFL_BOOL   allocated;
} CFL_LIST, *CFL_LISTP;

extern void cfl_list_init(CFL_LISTP list, CFL_UINT32 capacity);
extern CFL_LISTP cfl_list_new(CFL_UINT32 capacity);
extern CFL_LISTP cfl_list_newLen(CFL_UINT32 len);
extern void cfl_list_free(CFL_LISTP list);
extern void cfl_list_add(CFL_LISTP list, void *item);
extern void cfl_list_del(CFL_LISTP list, CFL_UINT32 ulIndex);
extern void cfl_list_delItem(CFL_LISTP list, void *item);
extern void *cfl_list_get(CFL_LISTP list, CFL_UINT32 ulIndex);
extern void cfl_list_set(CFL_LISTP list, CFL_UINT32 ulIndex, void *item);
extern void cfl_list_clear(CFL_LISTP list);
extern CFL_UINT32 cfl_list_length(CFL_LISTP list);
extern void cfl_list_setLength(CFL_LISTP list, CFL_UINT32 newLen);
extern CFL_LISTP cfl_list_clone(CFL_LISTP other);
extern void *cfl_list_remove(CFL_LISTP list, CFL_UINT32 ulIndex);
extern void *cfl_list_removeLast(CFL_LISTP list);

#ifdef __cplusplus
}
#endif

#endif
