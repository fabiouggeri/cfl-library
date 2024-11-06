#include <stdlib.h>

#include "cfl_map_str.h"

/**********************************************************************************************************************************/
/*                                                       MAPSTR_ENTRY API                                                         */
/**********************************************************************************************************************************/

/**
 *
 */
static void map_entry_init(CFL_MAPSTR_ENTRYP entry) {
   if (entry == NULL) {
      return;
   }
   cfl_str_init(&entry->key);
   cfl_str_init(&entry->value);
   entry->allocated = CFL_FALSE;
}

/**
 * 
 */
static void map_entry_free(CFL_MAPSTR_ENTRYP entry) {
   if (entry == NULL) {
      return;
   }
   cfl_str_free(&entry->key);
   cfl_str_free(&entry->value);
   if (entry->allocated) {
      CFL_MEM_FREE(entry);
   }
}

/**
 * 
 */
void cfl_mapstr_entry_setKey(CFL_MAPSTR_ENTRYP entry, const char *key) {
   cfl_str_setValue(&entry->key, key);
}

/**
 * 
 */
const char *cfl_mapstr_entry_getKey(CFL_MAPSTR_ENTRYP entry) {
   return cfl_str_getPtr(&entry->key);
}

/**
 * 
 */
CFL_STRP cfl_mapstr_entry_getKeyStr(CFL_MAPSTR_ENTRYP entry) {
   return &entry->key;
}

/**
 * 
 */
void cfl_mapstr_entry_setValue(CFL_MAPSTR_ENTRYP entry, const char *value) {
   cfl_str_setValue(&entry->value, value);
}

/**
 * 
 */
const char *cfl_mapstr_entry_getValue(CFL_MAPSTR_ENTRYP entry) {
   return cfl_str_getPtr(&entry->value);
}
/**
 * 
 */
CFL_STRP cfl_mapstr_entry_getValueStr(CFL_MAPSTR_ENTRYP entry) {
   return &entry->value;
}


static void freeMapEntries(CFL_ARRAYP entries) {
   CFL_UINT32 len = cfl_array_length(entries);
   CFL_UINT32 i;
   for (i = 0; i < len; i++) {
      map_entry_free((CFL_MAPSTR_ENTRYP) cfl_array_get(entries, i));
   }
}

/**********************************************************************************************************************************/
/*                                                          MAPSTR API                                                            */
/**********************************************************************************************************************************/
void cfl_mapstr_free(CFL_MAPSTRP map) {
   if (map == NULL) {
      return;
   }
   freeMapEntries(&map->entries);
   cfl_array_free(&map->entries);
   if (map->allocated) {
      CFL_MEM_FREE(map);
   }
}

/**
 * 
 */
void cfl_mapstr_init(CFL_MAPSTRP map) {
   if (map == NULL) {
      return;
   }
   cfl_array_init(&map->entries, 16, sizeof(CFL_MAPSTR_ENTRY));
   map->allocated = CFL_FALSE;
}

/**
 * 
 */
CFL_MAPSTRP cfl_mapstr_new(void) {
   CFL_MAPSTRP map = CFL_MEM_ALLOC(sizeof(CFL_MAPSTR));
   if (map != NULL) {
      cfl_mapstr_init(map);
      map->allocated = CFL_TRUE;
   }
   return map;
}


/**
 * 
 */
CFL_MAPSTR_ENTRYP cfl_mapstr_getEntry(CFL_MAPSTRP map, CFL_UINT32 index) {
   if (index >= cfl_array_length(&map->entries)) {
      return NULL;
   }
   return (CFL_MAPSTR_ENTRYP) cfl_array_get(&map->entries, index);
}

/**
 * 
 */
CFL_STRP cfl_mapstr_getStr(CFL_MAPSTRP map, const char *key) {
   CFL_UINT32 len = cfl_array_length(&map->entries);
   CFL_UINT32 i;
   for (i = 0; i < len; i++) {
      CFL_MAPSTR_ENTRYP entry = (CFL_MAPSTR_ENTRYP) cfl_array_get(&map->entries, i);
      if (cfl_str_bufferEquals(&entry->key, key)) {
         return &entry->value;
      }
   }
   return NULL;
}

/**
 * 
 */
const char * cfl_mapstr_get(CFL_MAPSTRP map, const char *key) {
   CFL_UINT32 len = cfl_array_length(&map->entries);
   CFL_UINT32 i;
   for (i = 0; i < len; i++) {
      CFL_MAPSTR_ENTRYP entry = (CFL_MAPSTR_ENTRYP) cfl_array_get(&map->entries, i);
      if (cfl_str_bufferEquals(&entry->key, key)) {
         return cfl_str_getPtr(&entry->value);
      }
   }
   return NULL;
}

const char *cfl_mapstr_getDefault(CFL_MAPSTRP map, const char *key, const char *defaultValue) {
   const char *value = cfl_mapstr_get(map, key);
   if (value != NULL) {
      return value;
   }
   return defaultValue;
}


const char *cfl_mapstr_getKeyIndex(CFL_MAPSTRP map, CFL_UINT32 index) {
   if (index >= 0 && index < cfl_array_length(&map->entries)) {
      CFL_MAPSTR_ENTRYP entry = (CFL_MAPSTR_ENTRYP) cfl_array_get(&map->entries, index);
      return cfl_str_getPtr(&entry->key);
   }
   return NULL;
}

const char *cfl_mapstr_getIndex(CFL_MAPSTRP map, CFL_UINT32 index) {
   if (index >= 0 && index < cfl_array_length(&map->entries)) {
      CFL_MAPSTR_ENTRYP entry = (CFL_MAPSTR_ENTRYP) cfl_array_get(&map->entries, index);
      return cfl_str_getPtr(&entry->value);
   }
   return NULL;
}

CFL_STRP cfl_mapstr_getKeyStrIndex(CFL_MAPSTRP map, CFL_UINT32 index) {
   if (index >= 0 && index < cfl_array_length(&map->entries)) {
      CFL_MAPSTR_ENTRYP entry = (CFL_MAPSTR_ENTRYP) cfl_array_get(&map->entries, index);
      return &entry->key;
   }
   return NULL;
}

CFL_STRP cfl_mapstr_getStrIndex(CFL_MAPSTRP map, CFL_UINT32 index) {
   if (index >= 0 && index < cfl_array_length(&map->entries)) {
      CFL_MAPSTR_ENTRYP entry = (CFL_MAPSTR_ENTRYP) cfl_array_get(&map->entries, index);
      return &entry->value;
   }
   return NULL;
}

/**
 * 
 */
CFL_BOOL cfl_mapstr_del(CFL_MAPSTRP map, const char *key) {
   CFL_UINT32 len = cfl_array_length(&map->entries);
   CFL_UINT32 i;
   for (i = 0; i < len; i++) {
      CFL_MAPSTR_ENTRYP entry = (CFL_MAPSTR_ENTRYP) cfl_array_get(&map->entries, i);
      if (cfl_str_bufferEquals(&entry->key, key)) {
         map_entry_free(entry);
         cfl_array_del(&map->entries, i);
         return CFL_TRUE;
      }
   }
   return CFL_FALSE;
}

/**
 * 
 */
void cfl_mapstr_set(CFL_MAPSTRP map, const char *key, const char *value) {
   CFL_MAPSTR_ENTRYP entry;
   CFL_UINT32 len = cfl_array_length(&map->entries);
   CFL_UINT32 i;
   for (i = 0; i < len; i++) {
      entry = (CFL_MAPSTR_ENTRYP) cfl_array_get(&map->entries, i);
      if (cfl_str_bufferEquals(&entry->key, key)) {
         cfl_str_setValue(&entry->value, value);
         return;
      }
   }
   entry = (CFL_MAPSTR_ENTRYP) cfl_array_add(&map->entries);
   map_entry_init(entry);
   cfl_str_setValue(&entry->key, key);
   cfl_str_setValue(&entry->value, value);
}

void cfl_mapstr_setStr(CFL_MAPSTRP map, CFL_STRP key, CFL_STRP value) {
   CFL_MAPSTR_ENTRYP entry;
   CFL_UINT32 len = cfl_array_length(&map->entries);
   CFL_UINT32 i;
   for (i = 0; i < len; i++) {
      entry = (CFL_MAPSTR_ENTRYP) cfl_array_get(&map->entries, i);
      if (cfl_str_equals(&entry->key, key)) {
         cfl_str_setStr(&entry->value, value);
         return;
      }
   }
   entry = (CFL_MAPSTR_ENTRYP) cfl_array_add(&map->entries);
   map_entry_init(entry);
   cfl_str_setStr(&entry->key, key);
   cfl_str_setStr(&entry->value, value);
}

/**
 * 
 */
void cfl_mapstr_setFormat(CFL_MAPSTRP map, const char *key, const char *format, ...) {
   CFL_MAPSTR_ENTRYP entry;
   CFL_UINT32 len = cfl_array_length(&map->entries);
   CFL_UINT32 i;
   va_list varArgs;

   va_start(varArgs, format);

   for (i = 0; i < len; i++) {
      entry = (CFL_MAPSTR_ENTRYP) cfl_array_get(&map->entries, i);
      if (cfl_str_bufferEquals(&entry->key, key)) {
         cfl_str_setFormatArgs(&entry->value, format, varArgs);
         va_end(varArgs);
         return;
      }
   }
   entry = (CFL_MAPSTR_ENTRYP) cfl_array_add(&map->entries);
   map_entry_init(entry);
   cfl_str_setValue(&entry->key, key);
   cfl_str_setFormatArgs(&entry->value, format, varArgs);
   va_end(varArgs);
}

/**
 * 
 */
void cfl_mapstr_copy(CFL_MAPSTRP toMap, CFL_MAPSTRP fromMap) {
   CFL_UINT32 len = cfl_array_length(&fromMap->entries);
   CFL_UINT32 i;
   for (i = 0; i < len; i++) {
      CFL_MAPSTR_ENTRYP fromEntry = (CFL_MAPSTR_ENTRYP) cfl_array_get(&fromMap->entries, i);
      CFL_MAPSTR_ENTRYP toEntry = (CFL_MAPSTR_ENTRYP) cfl_array_add(&toMap->entries);
      map_entry_init(toEntry);
      cfl_str_setStr(&toEntry->key, &fromEntry->key);
      cfl_str_setStr(&toEntry->value, &fromEntry->value);
   }
}

/**
 * 
 */
CFL_UINT32 cfl_mapstr_length(CFL_MAPSTRP map) {
   return cfl_array_length(&map->entries);
}
