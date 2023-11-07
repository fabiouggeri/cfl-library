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

#ifndef CFL_ARRAY_H_

#define CFL_ARRAY_H_

#include "cfl_types.h"


#ifdef __cplusplus
extern "C" {
#endif

struct _CFL_ARRAY {
   CFL_UINT8  *items;
   CFL_UINT32 ulItemSize;
   CFL_UINT32 ulLength;
   CFL_UINT32 ulCapacity;
   CFL_BOOL   allocated;
};

extern void cfl_array_init(CFL_ARRAYP array, CFL_UINT32 ulCapacity, CFL_UINT32 ulItemSize);
extern CFL_ARRAYP cfl_array_new(CFL_UINT32 ulCapacity, CFL_UINT32 ulItemSize);
extern CFL_ARRAYP cfl_array_newLen(CFL_UINT32 ulLen, CFL_UINT32 ulItemSize);
extern void cfl_array_free(CFL_ARRAYP array);
extern void *cfl_array_add(CFL_ARRAYP array);
extern void *cfl_array_insert(CFL_ARRAYP array, CFL_UINT32 ulIndex);
extern void cfl_array_del(CFL_ARRAYP array, CFL_UINT32 ulIndex);
extern void *cfl_array_get(CFL_ARRAYP array, CFL_UINT32 ulIndex);
extern CFL_UINT32 cfl_array_getItemPos(CFL_ARRAYP array, void *item);
extern void cfl_array_set(CFL_ARRAYP array, CFL_UINT32 ulIndex, void *item);
extern void cfl_array_clear(CFL_ARRAYP array);
extern CFL_UINT32 cfl_array_length(CFL_ARRAYP array);
extern void cfl_array_setLength(CFL_ARRAYP array, CFL_UINT32 newLen);
extern CFL_ARRAYP cfl_array_clone(CFL_ARRAYP other);
extern CFL_ITERATORP cfl_array_iterator(CFL_ARRAYP array);

#ifdef __cplusplus
}
#endif

#endif
