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

#include <string.h>
#include <stdlib.h>

#include "cfl_map.h"
#include "cfl_mem.h"

#define GET_KEY(e)         ((void *) (e)->data)
#define GET_VALUE(m, e)    ((void *) &((e)->data[(m)->keySize]))
#define SET_KEY(m, e, k)   memcpy((e)->data, k, (m)->keySize)
#define SET_VALUE(m, e, v) memcpy(&((e)->data[(m)->keySize]), v, (m)->valueSize)

typedef struct _CFL_MAP_ENTRY {
   char data[1];
} CFL_MAP_ENTRY, *CFL_MAP_ENTRYP;

static void freeMapEntries(CFL_MAPP map) {
   CFL_UINT32 len = cfl_array_length(&map->entries);
   CFL_UINT32 i;
   for (i = 0; i < len; i++) {
      CFL_MAP_ENTRYP entry = (CFL_MAP_ENTRYP) cfl_array_get(&map->entries, i);
      map->freeEntryFunc(GET_KEY(entry), GET_VALUE(map, entry));
   }
}

/**********************************************************************************************************************************/
/*                                                            MAP API                                                             */
/**********************************************************************************************************************************/
void cfl_map_free(CFL_MAPP map) {
   if (map == NULL) {
      return;
   }
   freeMapEntries(map);
   cfl_array_free(&map->entries);
   if (map->allocated) {
      CFL_MEM_FREE(map);
   }
}

/**
 * 
 */
void cfl_map_init(CFL_MAPP map, CFL_UINT32 keySize, CFL_UINT32 valueSize, MAP_COMP_FUNC keyCompFunc, MAP_KEY_VALUE_FUNC freeEntryFunc) {
   if (map == NULL) {
      return;
   }
   cfl_array_init(&map->entries, 16, keySize + valueSize);
   map->keySize = keySize;
   map->valueSize = valueSize;
   map->keyCompFunc = keyCompFunc;
   map->freeEntryFunc = freeEntryFunc;
   map->allocated = CFL_FALSE;
}

/**
 * 
 */
CFL_MAPP cfl_map_new(CFL_UINT32 keySize, CFL_UINT32 valueSize, MAP_COMP_FUNC keyCompFunc, MAP_KEY_VALUE_FUNC freeEntryFunc) {
   CFL_MAPP map = CFL_MEM_ALLOC(sizeof(CFL_MAP));
   if (map != NULL) {
      cfl_map_init(map, keySize, valueSize, keyCompFunc, freeEntryFunc);
      map->allocated = CFL_TRUE;
   }
   return map;
}

/**
 * 
 */
const void * cfl_map_get(CFL_MAPP map, const void *key) {
   CFL_UINT32 len = cfl_array_length(&map->entries);
   CFL_UINT32 i;
   for (i = 0; i < len; i++) {
      CFL_MAP_ENTRYP entry = (CFL_MAP_ENTRYP) cfl_array_get(&map->entries, i);
      void *value = GET_VALUE(map, entry);
      if (map->keyCompFunc(GET_KEY(entry), key) == 0) {
         return value;
      }
   }
   return NULL;
}

const void *cfl_map_getIndex(CFL_MAPP map, CFL_UINT32 index) {
   if (index < cfl_array_length(&map->entries)) {
      return GET_VALUE(map, (CFL_MAP_ENTRYP) cfl_array_get(&map->entries, index));
   }
   return NULL;
}

/**
 * 
 */
CFL_BOOL cfl_map_del(CFL_MAPP map, const void *key) {
   CFL_UINT32 len = cfl_array_length(&map->entries);
   CFL_UINT32 i;
   for (i = 0; i < len; i++) {
      CFL_MAP_ENTRYP entry = (CFL_MAP_ENTRYP) cfl_array_get(&map->entries, i);
      void *entryKey = GET_KEY(entry);
      if (map->keyCompFunc(entryKey, key) == 0) {
         map->freeEntryFunc(entryKey, key);
         cfl_array_del(&map->entries, i);
         return CFL_TRUE;
      }
   }
   return CFL_FALSE;
}

/**
 * 
 */
void cfl_map_set(CFL_MAPP map, const void *newKey, const void *newValue) {
   CFL_MAP_ENTRYP entry;
   CFL_UINT32 len = cfl_array_length(&map->entries);
   CFL_UINT32 i;
   for (i = 0; i < len; i++) {
      void *key;
      void *value;
      entry = (CFL_MAP_ENTRYP) cfl_array_get(&map->entries, i);
      key = GET_KEY(entry);
      value = GET_VALUE(map, entry);
      if (map->keyCompFunc(key, value) == 0) {
         SET_VALUE(map, entry, newValue);
         return;
      }
   }
   entry = (CFL_MAP_ENTRYP) cfl_array_add(&map->entries);
   SET_KEY(map, entry, newKey);
   SET_VALUE(map, entry, newValue);
}

/**
 * 
 */
void cfl_map_copy(CFL_MAPP toMap, CFL_MAPP fromMap) {
   CFL_UINT32 len = cfl_array_length(&fromMap->entries);
   CFL_UINT32 i;
   if (toMap->keySize != fromMap->keySize || toMap->valueSize != fromMap->valueSize) {
      return;
   }
   freeMapEntries(toMap);
   cfl_array_clear(&toMap->entries);
   for (i = 0; i < len; i++) {
      CFL_MAP_ENTRYP fromEntry = (CFL_MAP_ENTRYP) cfl_array_get(&fromMap->entries, i);
      CFL_MAP_ENTRYP toEntry = (CFL_MAP_ENTRYP) cfl_array_add(&toMap->entries);
      SET_KEY(toMap, toEntry, GET_KEY(fromEntry));
      SET_VALUE(toMap, toEntry, GET_VALUE(fromMap, fromEntry));
   }
}

/**
 * 
 */
CFL_UINT32 cfl_map_length(CFL_MAPP map) {
   return cfl_array_length(&map->entries);
}

