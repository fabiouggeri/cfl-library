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

#ifndef CFL_MAP_STR_H_

#define CFL_MAP_STR_H_

#include "cfl_types.h"

#include "cfl_str.h"
#include "cfl_array.h"

#ifdef __cplusplus
extern "C" {
#endif

struct _CFL_MAPSTR_ENTRY {
   CFL_STR  key;
   CFL_STR  value;
   CFL_BOOL allocated;
};

struct _CFL_MAPSTR {
   CFL_ARRAY entries;
   CFL_BOOL  allocated;
};

extern void cfl_mapstr_entry_setKey(CFL_MAPSTR_ENTRYP entry, const char *key);
extern const char *cfl_mapstr_entry_getKey(CFL_MAPSTR_ENTRYP entry);
extern CFL_STRP cfl_mapstr_entry_getKeyStr(CFL_MAPSTR_ENTRYP entry);
extern void cfl_mapstr_entry_setValue(CFL_MAPSTR_ENTRYP entry, const char *value);
extern const char *cfl_mapstr_entry_getValue(CFL_MAPSTR_ENTRYP entry);
extern CFL_STRP cfl_mapstr_entry_getValueStr(CFL_MAPSTR_ENTRYP entry);

extern void cfl_mapstr_init(CFL_MAPSTRP map);
extern CFL_MAPSTRP cfl_mapstr_new(void);
extern void cfl_mapstr_free(CFL_MAPSTRP map);
extern CFL_MAPSTR_ENTRYP cfl_mapstr_getEntry(CFL_MAPSTRP map, CFL_UINT32 index);
extern const char *cfl_mapstr_getKeyIndex(CFL_MAPSTRP map, CFL_UINT32 index);
extern const char *cfl_mapstr_get(CFL_MAPSTRP map, const char *key);
extern const char *cfl_mapstr_getIndex(CFL_MAPSTRP map, CFL_UINT32 index);
extern CFL_STRP cfl_mapstr_getStr(CFL_MAPSTRP map, const char *key);
extern CFL_STRP cfl_mapstr_getStrIndex(CFL_MAPSTRP map, CFL_UINT32 index);
extern CFL_BOOL cfl_mapstr_del(CFL_MAPSTRP map, const char *key);
extern void cfl_mapstr_set(CFL_MAPSTRP map, const char *key, const char *value);
extern void cfl_mapstr_setStr(CFL_MAPSTRP map, CFL_STRP key, CFL_STRP value);
extern void cfl_mapstr_setFormat(CFL_MAPSTRP map, const char *key, const char *format, ...);
extern void cfl_mapstr_copy(CFL_MAPSTRP toMap, CFL_MAPSTRP fromMap);
extern CFL_UINT32 cfl_mapstr_length(CFL_MAPSTRP map);

#ifdef __cplusplus
}
#endif

#endif
