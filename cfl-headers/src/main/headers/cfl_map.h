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

#ifndef CFL_MAP_H_

#define CFL_MAP_H_

#include "cfl_types.h"
#include "cfl_array.h"

typedef int  (*MAP_COMP_FUNC) (void *k1, void *k2);
typedef void (*MAP_KEY_VALUE_FUNC) (void *k, void *v);

struct _CFL_MAP {
   CFL_ARRAY          entries;
   CFL_UINT32         keySize;
   CFL_UINT32         valueSize;
   MAP_COMP_FUNC      keyCompFunc;
   MAP_KEY_VALUE_FUNC freeEntryFunc;
   CFL_BOOL           allocated;
};

extern void cfl_map_init(CFL_MAPP map, CFL_UINT32 keySize, CFL_UINT32 valueSize, MAP_COMP_FUNC keyCompFunc, MAP_KEY_VALUE_FUNC freeEntryFunc);
extern CFL_MAPP cfl_map_new(CFL_UINT32 keySize, CFL_UINT32 valueSize, MAP_COMP_FUNC keyCompFunc, MAP_KEY_VALUE_FUNC freeEntryFunc);
extern void cfl_map_free(CFL_MAPP map);
extern const void *cfl_map_get(CFL_MAPP map, const void *key);
extern const void *cfl_map_getIndex(CFL_MAPP map, CFL_UINT32 index);
extern const void *cfl_map_getKeyIndex(CFL_MAPP map, CFL_UINT32 index);
extern CFL_BOOL cfl_map_del(CFL_MAPP map, const void *key);
extern void cfl_map_set(CFL_MAPP map, const void *key, const void *value);
extern void cfl_map_setIndex(CFL_MAPP map, CFL_UINT32 index, const void *value);
extern void cfl_map_setKeyIndex(CFL_MAPP map, CFL_UINT32 index, const void *key);
extern void cfl_map_copy(CFL_MAPP toMap, CFL_MAPP fromMap);
extern CFL_UINT32 cfl_map_length(CFL_MAPP map);


#endif
