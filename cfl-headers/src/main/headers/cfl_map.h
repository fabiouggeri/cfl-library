/**
 * @file cfl_map.h
 * @brief Generic key-value map implementation using arrays.
 *
 * This module provides a map (dictionary) data structure that stores key-value
 * pairs with O(n) lookup complexity. Suitable for small collections.
 */

#ifndef CFL_MAP_H_

#define CFL_MAP_H_

#include "cfl_array.h"
#include "cfl_types.h"


#ifdef __cplusplus
extern "C" {
#endif

/** @brief Function pointer type for comparing keys */
typedef int (*MAP_COMP_FUNC)(const void *k1, const void *k2);
/** @brief Function pointer type for freeing entries */
typedef void (*MAP_KEY_VALUE_FUNC)(const void *k, const void *v);

/**
 * @brief Map structure for storing key-value pairs.
 */
typedef struct _CFL_MAP {
  CFL_ARRAY entries;                /**< Internal array of entries */
  CFL_UINT32 keySize;               /**< Size of each key in bytes */
  CFL_UINT32 valueSize;             /**< Size of each value in bytes */
  MAP_COMP_FUNC keyCompFunc;        /**< Key comparison function */
  MAP_KEY_VALUE_FUNC freeEntryFunc; /**< Entry cleanup function */
  CFL_BOOL allocated;               /**< Whether structure was allocated */
} CFL_MAP, *CFL_MAPP;

/**
 * @brief Initializes a map structure.
 * @param map Pointer to the map to initialize.
 * @param keySize Size of each key in bytes.
 * @param valueSize Size of each value in bytes.
 * @param keyCompFunc Function to compare keys.
 * @param freeEntryFunc Function to free entries (can be NULL).
 */
extern void cfl_map_init(CFL_MAPP map, CFL_UINT32 keySize, CFL_UINT32 valueSize,
                         MAP_COMP_FUNC keyCompFunc,
                         MAP_KEY_VALUE_FUNC freeEntryFunc);

/**
 * @brief Creates a new map.
 * @param keySize Size of each key in bytes.
 * @param valueSize Size of each value in bytes.
 * @param keyCompFunc Function to compare keys.
 * @param freeEntryFunc Function to free entries (can be NULL).
 * @return Pointer to the new map, or NULL if allocation fails.
 */
extern CFL_MAPP cfl_map_new(CFL_UINT32 keySize, CFL_UINT32 valueSize,
                            MAP_COMP_FUNC keyCompFunc,
                            MAP_KEY_VALUE_FUNC freeEntryFunc);

/**
 * @brief Frees the memory used by a map.
 * @param map Pointer to the map to free.
 */
extern void cfl_map_free(CFL_MAPP map);

/**
 * @brief Gets the value associated with a key.
 * @param map Pointer to the map.
 * @param key Pointer to the key to search for.
 * @return Pointer to the value, or NULL if key not found.
 */
extern const void *cfl_map_get(CFL_MAPP map, const void *key);

/**
 * @brief Gets the value at a specific index.
 * @param map Pointer to the map.
 * @param index Index of the entry.
 * @return Pointer to the value, or NULL if index is out of bounds.
 */
extern const void *cfl_map_getIndex(CFL_MAPP map, CFL_UINT32 index);

/**
 * @brief Gets the key at a specific index.
 * @param map Pointer to the map.
 * @param index Index of the entry.
 * @return Pointer to the key, or NULL if index is out of bounds.
 */
extern const void *cfl_map_getKeyIndex(CFL_MAPP map, CFL_UINT32 index);

/**
 * @brief Deletes an entry by key.
 * @param map Pointer to the map.
 * @param key Pointer to the key to delete.
 * @return CFL_TRUE if key was found and deleted, CFL_FALSE otherwise.
 */
extern CFL_BOOL cfl_map_del(CFL_MAPP map, const void *key);

/**
 * @brief Sets or updates a key-value pair.
 * @param map Pointer to the map.
 * @param key Pointer to the key.
 * @param value Pointer to the value.
 * @note If key exists, the value is updated; otherwise, a new entry is added.
 */
extern void cfl_map_set(CFL_MAPP map, const void *key, const void *value);

/**
 * @brief Sets the value at a specific index.
 * @param map Pointer to the map.
 * @param index Index of the entry.
 * @param value Pointer to the new value.
 */
extern void cfl_map_setIndex(CFL_MAPP map, CFL_UINT32 index, const void *value);

/**
 * @brief Sets the key at a specific index.
 * @param map Pointer to the map.
 * @param index Index of the entry.
 * @param key Pointer to the new key.
 */
extern void cfl_map_setKeyIndex(CFL_MAPP map, CFL_UINT32 index,
                                const void *key);

/**
 * @brief Copies all entries from one map to another.
 * @param toMap Destination map.
 * @param fromMap Source map.
 * @note Maps must have same key and value sizes.
 */
extern void cfl_map_copy(CFL_MAPP toMap, CFL_MAPP fromMap);

/**
 * @brief Returns the number of entries in the map.
 * @param map Pointer to the map.
 * @return Number of key-value pairs.
 */
extern CFL_UINT32 cfl_map_length(CFL_MAPP map);

#ifdef __cplusplus
}
#endif

#endif
