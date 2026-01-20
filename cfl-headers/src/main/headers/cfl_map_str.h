/**
 * @file cfl_map_str.h
 * @brief String-to-String Map implementation.
 *
 * This module provides a specialized map implementation where both keys and
 * values are strings. It keeps copies of the strings, making it easier to
 * manage memory for simple string dictionaries.
 */

#ifndef CFL_MAP_STR_H_

#define CFL_MAP_STR_H_

#include "cfl_types.h"

#include "cfl_array.h"
#include "cfl_str.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Entry structure for String Map.
 */
typedef struct _CFL_MAPSTR_ENTRY {
  CFL_STR key;        /**< Key string */
  CFL_STR value;      /**< Value string */
  CFL_BOOL allocated; /**< Allocation flag */
} CFL_MAPSTR_ENTRY, *CFL_MAPSTR_ENTRYP;

/**
 * @brief String Map structure.
 */
typedef struct _CFL_MAPSTR {
  CFL_ARRAY entries;  /**< Array of CFL_MAPSTR_ENTRY */
  CFL_BOOL allocated; /**< Allocation flag */
} CFL_MAPSTR, *CFL_MAPSTRP;

/**
 * @brief Sets the key for a map entry from a C-string.
 * @param entry The entry.
 * @param key The key C-string.
 */
extern void cfl_mapstr_entry_setKey(CFL_MAPSTR_ENTRYP entry, const char *key);

/**
 * @brief Gets the key of a map entry as a C-string.
 * @param entry The entry.
 * @return The key C-string.
 */
extern const char *cfl_mapstr_entry_getKey(CFL_MAPSTR_ENTRYP entry);

/**
 * @brief Gets the key of a map entry as a CFL_STRP.
 * @param entry The entry.
 * @return The key CFL_STRP.
 */
extern CFL_STRP cfl_mapstr_entry_getKeyStr(CFL_MAPSTR_ENTRYP entry);

/**
 * @brief Sets the value for a map entry from a C-string.
 * @param entry The entry.
 * @param value The value C-string.
 */
extern void cfl_mapstr_entry_setValue(CFL_MAPSTR_ENTRYP entry,
                                      const char *value);

/**
 * @brief Gets the value of a map entry as a C-string.
 * @param entry The entry.
 * @return The value C-string.
 */
extern const char *cfl_mapstr_entry_getValue(CFL_MAPSTR_ENTRYP entry);

/**
 * @brief Gets the value of a map entry as a CFL_STRP.
 * @param entry The entry.
 * @return The value CFL_STRP.
 */
extern CFL_STRP cfl_mapstr_entry_getValueStr(CFL_MAPSTR_ENTRYP entry);

/**
 * @brief Initializes a String Map.
 * @param map The map to initialize.
 */
extern void cfl_mapstr_init(CFL_MAPSTRP map);

/**
 * @brief Creates a new String Map.
 * @return Pointer to the new map.
 */
extern CFL_MAPSTRP cfl_mapstr_new(void);

/**
 * @brief Frees a String Map and all its contents.
 * @param map The map to free.
 */
extern void cfl_mapstr_free(CFL_MAPSTRP map);

/**
 * @brief Gets the entry at a specific index.
 * @param map The map.
 * @param index The index.
 * @return Pointer to the entry, or NULL if out of bounds.
 */
extern CFL_MAPSTR_ENTRYP cfl_mapstr_getEntry(CFL_MAPSTRP map, CFL_UINT32 index);

/**
 * @brief Gets the key at a specific index.
 * @param map The map.
 * @param index The index.
 * @return The key as C-string.
 */
extern const char *cfl_mapstr_getKeyIndex(CFL_MAPSTRP map, CFL_UINT32 index);

/**
 * @brief Gets the value associated with a key (C-string lookup).
 * @param map The map.
 * @param key The key to search for.
 * @return The value as C-string, or NULL if not found.
 */
extern const char *cfl_mapstr_get(CFL_MAPSTRP map, const char *key);

/**
 * @brief Gets the value or a default if null/not found.
 * @param map The map.
 * @param key The key to search for.
 * @param defaultValue Value to return if key not found or value is null.
 * @return The value or default.
 */
extern const char *cfl_mapstr_getDefault(CFL_MAPSTRP map, const char *key,
                                         const char *defaultValue);

/**
 * @brief Gets the value at a specific index as C-string.
 * @param map The map.
 * @param index The index.
 * @return The value as C-string.
 */
extern const char *cfl_mapstr_getIndex(CFL_MAPSTRP map, CFL_UINT32 index);

/**
 * @brief Gets the value associated with a key (C-string lookup) as CFL_STRP.
 * @param map The map.
 * @param key The key to search for.
 * @return The value as CFL_STRP, or NULL if not found.
 */
extern CFL_STRP cfl_mapstr_getStr(CFL_MAPSTRP map, const char *key);

/**
 * @brief Gets the key at a specific index as CFL_STRP.
 * @param map The map.
 * @param index The index.
 * @return The key as CFL_STRP.
 */
extern CFL_STRP cfl_mapstr_getKeyStrIndex(CFL_MAPSTRP map, CFL_UINT32 index);

/**
 * @brief Gets the value at a specific index as CFL_STRP.
 * @param map The map.
 * @param index The index.
 * @return The value as CFL_STRP.
 */
extern CFL_STRP cfl_mapstr_getStrIndex(CFL_MAPSTRP map, CFL_UINT32 index);

/**
 * @brief Deletes an entry by key.
 * @param map The map.
 * @param key The key to delete.
 * @return CFL_TRUE if found and deleted, CFL_FALSE otherwise.
 */
extern CFL_BOOL cfl_mapstr_del(CFL_MAPSTRP map, const char *key);

/**
 * @brief Sets a key-value pair using C-strings.
 * @param map The map.
 * @param key The key.
 * @param value The value.
 */
extern void cfl_mapstr_set(CFL_MAPSTRP map, const char *key, const char *value);

/**
 * @brief Sets a key-value pair using CFL_STRPs.
 * @param map The map.
 * @param key The key.
 * @param value The value.
 */
extern void cfl_mapstr_setStr(CFL_MAPSTRP map, CFL_STRP key, CFL_STRP value);

/**
 * @brief Sets a key-value pair using printf-style formatting for the value.
 * @param map The map.
 * @param key The key.
 * @param format Format string.
 * @param ... Arguments.
 */
extern void cfl_mapstr_setFormat(CFL_MAPSTRP map, const char *key,
                                 const char *format, ...);

/**
 * @brief Copies contents from one map to another.
 * @param toMap Destination map.
 * @param fromMap Source map.
 */
extern void cfl_mapstr_copy(CFL_MAPSTRP toMap, CFL_MAPSTRP fromMap);

/**
 * @brief Gets the number of entries in the map.
 * @param map The map.
 * @return count of entries.
 */
extern CFL_UINT32 cfl_mapstr_length(CFL_MAPSTRP map);

#ifdef __cplusplus
}
#endif

#endif
