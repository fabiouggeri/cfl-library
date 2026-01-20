/**
 * @file cfl_hash.h
 * @brief Hash table implementation.
 *
 * This module provides a generic hash table implementation for storing
 * key-value pairs. It supports custom hash, equality, and free functions.
 */

#ifndef CFL_HASH_H_

#define CFL_HASH_H_

#include "cfl_iterator.h"
#include "cfl_types.h"


#ifdef __cplusplus
extern "C" {
#endif

struct _CFL_HASH_ENTRY;
typedef struct _CFL_HASH_ENTRY CFL_HASH_ENTRY;
typedef CFL_HASH_ENTRY *CFL_HASH_ENTRYP;

/** @brief Function pointer for calculating hash of a key */
typedef CFL_UINT32 (*HASH_KEY_FUNC)(void *k);
/** @brief Function pointer for comparing two keys for equality */
typedef int (*HASH_COMP_FUNC)(void *k1, void *k2);
/** @brief Function pointer for freeing key and value pairs */
typedef void (*HASH_FREE_FUNC)(void *k, void *v);

/**
 * @brief Hash table entry structure.
 */
struct _CFL_HASH_ENTRY {
  void *key;            /**< Key pointer */
  void *value;          /**< Value pointer */
  CFL_HASH_ENTRYP next; /**< Pointer to next entry in the bucket */
  CFL_UINT32 hash;      /**< Cached hash value */
};

/**
 * @brief Hash table structure.
 */
typedef struct _CFL_HASH {
  CFL_HASH_ENTRY **table; /**< Array of buckets */
  HASH_KEY_FUNC hashfn;   /**< Hash calculation function */
  HASH_COMP_FUNC eqfn;    /**< Key equality function */
  HASH_FREE_FUNC freefn;  /**< Entry free function */
  CFL_UINT32 tablelength; /**< Current size of the table */
  CFL_UINT32 entrycount;  /**< Number of entries in the table */
  CFL_UINT32 loadlimit;   /**< Threshold to expand the table */
  CFL_UINT32 primeindex;  /**< Index in prime number table for sizing */
} CFL_HASH, *CFL_HASHP;

/**
 * @brief Creates a new hash table.
 *
 * @param minsize Minimum initial size of the hash table.
 * @param hashf Function for hashing keys.
 * @param eqf Function for determining key equality.
 * @param freef Function for freeing keys and values (can be NULL).
 * @return Pointer to the newly created hash table, or NULL on failure.
 */
extern CFL_HASHP cfl_hash_new(CFL_UINT32 minsize, HASH_KEY_FUNC hashf,
                              HASH_COMP_FUNC eqf, HASH_FREE_FUNC freef);

/**
 * @brief Inserts a key-value pair into the hash table.
 *
 * This function will cause the table to expand if the insertion would take
 * the ratio of entries to table size over the maximum load factor.
 *
 * @param h The hash table to insert into.
 * @param k The key - hashtable claims ownership and will free on
 * removal/destruction if freefn is provided.
 * @param v The value - hashtable claims ownership and will free on
 * removal/destruction if freefn is provided.
 * @return Non-zero for successful insertion, 0 on failure.
 *
 * @note This function does not check for repeated insertions with a duplicate
 * key. Using a duplicate key results in undefined retrieval order. If in doubt,
 * remove before insert.
 */
extern int cfl_hash_insert(CFL_HASHP h, void *k, void *v);

/**
 * @brief Searches for a value by key.
 *
 * @param h The hash table to search.
 * @param k The key to search for (does not claim ownership).
 * @return The value associated with the key, or NULL if none found.
 */
extern void *cfl_hash_search(CFL_HASHP h, void *k);

/**
 * @brief Removes an entry from the hash table.
 *
 * @param h The hash table to remove the item from.
 * @param k The key to search for (does not claim ownership).
 * @return The value associated with the key, or NULL if none found.
 * @note The key and value are NOT freed by this function, the caller takes
 * ownership. However, the internal entry structure is freed.
 */
extern void *cfl_hash_remove(CFL_HASHP h, void *k);

/**
 * @brief Returns the number of items in the hash table.
 *
 * @param h The hash table.
 * @return The number of items stored.
 */
extern CFL_UINT32 cfl_hash_count(const CFL_HASHP h);

/**
 * @brief Frees the hash table and all its resources.
 *
 * @param h The hash table to free.
 * @param free_values If CFL_TRUE, calls 'free' loop on remaining values
 * (deprecated param name, uses freefn).
 */
extern void cfl_hash_free(CFL_HASHP h, CFL_BOOL free_values);

/**
 * @brief Clears all entries from the hash table.
 *
 * @param h The hash table to clear.
 * @param free_values If CFL_TRUE, calls 'free' loop on remaining values
 * (deprecated param name, uses freefn).
 */
extern void cfl_hash_clear(CFL_HASHP h, CFL_BOOL free_values);

/**
 * @brief Calculates the hash for a key using the table's hash function.
 * @param h The hash table.
 * @param k The key to hash.
 * @return The calculated hash value.
 */
CFL_UINT32 cfl_hash_calc(CFL_HASHP h, void *k);

/**
 * @brief Helper function to compute Murmur3 hash.
 * @param key Pointer to data to hash.
 * @param len Length of the data in bytes.
 * @return The computed hash value.
 */
CFL_UINT32 cfl_hash_murmur3(const void *key, CFL_UINT32 len);

/**
 * @brief Creates an iterator for the hash table.
 * @param h The hash table to iterate over.
 * @return Pointer to the new iterator.
 */
CFL_ITERATORP cfl_hash_iterator(CFL_HASHP h);

#ifdef __cplusplus
}
#endif

#endif
