/**
 * @file cfl_array.h
 * @brief Generic dynamic array implementation for fixed-size elements.
 *
 * This module provides a dynamic array data structure that stores fixed-size
 * elements contiguously in memory. It supports automatic resizing when elements
 * are added.
 */

#ifndef CFL_ARRAY_H_

#define CFL_ARRAY_H_

#include "cfl_iterator.h"
#include "cfl_types.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Dynamic array structure for fixed-size elements.
 */
typedef struct _CFL_ARRAY {
  CFL_UINT8 *items;      /**< Pointer to the array data storage */
  CFL_UINT32 ulItemSize; /**< Size of each element in bytes */
  CFL_UINT32 ulLength;   /**< Current number of elements in the array */
  CFL_UINT32 ulCapacity; /**< Maximum number of elements before reallocation */
  CFL_BOOL allocated;    /**< Whether the structure was dynamically allocated */
} CFL_ARRAY, *CFL_ARRAYP;

/**
 * @brief Initializes an array structure with the given capacity and item size.
 * @param array Pointer to the array structure to initialize.
 * @param ulCapacity Initial capacity (number of elements).
 * @param ulItemSize Size of each element in bytes.
 */
extern void cfl_array_init(CFL_ARRAYP array, CFL_UINT32 ulCapacity,
                           CFL_UINT32 ulItemSize);

/**
 * @brief Creates a new array with the given capacity and item size.
 * @param ulCapacity Initial capacity (number of elements).
 * @param ulItemSize Size of each element in bytes.
 * @return Pointer to the new array, or NULL if allocation fails.
 */
extern CFL_ARRAYP cfl_array_new(CFL_UINT32 ulCapacity, CFL_UINT32 ulItemSize);

/**
 * @brief Creates a new array with the given length and item size,
 * pre-allocating elements.
 * @param ulLen Initial length (number of elements).
 * @param ulItemSize Size of each element in bytes.
 * @return Pointer to the new array, or NULL if allocation fails.
 */
extern CFL_ARRAYP cfl_array_newLen(CFL_UINT32 ulLen, CFL_UINT32 ulItemSize);

/**
 * @brief Frees the memory used by an array.
 * @param array Pointer to the array to free.
 */
extern void cfl_array_free(CFL_ARRAYP array);

/**
 * @brief Adds a new element to the end of the array.
 * @param array Pointer to the array.
 * @return Pointer to the newly added element slot, or NULL on failure.
 * @note The returned pointer should be used to set the element value.
 */
extern void *cfl_array_add(CFL_ARRAYP array);

/**
 * @brief Inserts a new element at the specified index.
 * @param array Pointer to the array.
 * @param ulIndex Index at which to insert the element.
 * @return Pointer to the inserted element slot, or NULL on failure.
 */
extern void *cfl_array_insert(CFL_ARRAYP array, CFL_UINT32 ulIndex);

/**
 * @brief Deletes the element at the specified index.
 * @param array Pointer to the array.
 * @param ulIndex Index of the element to delete.
 */
extern void cfl_array_del(CFL_ARRAYP array, CFL_UINT32 ulIndex);

/**
 * @brief Gets the element at the specified index.
 * @param array Pointer to the array.
 * @param ulIndex Index of the element to retrieve.
 * @return Pointer to the element, or NULL if index is out of bounds.
 */
extern void *cfl_array_get(const CFL_ARRAYP array, CFL_UINT32 ulIndex);

/**
 * @brief Gets the index position of an item in the array.
 * @param array Pointer to the array.
 * @param item Pointer to the item.
 * @return Index of the item, or array length if not found.
 */
extern CFL_UINT32 cfl_array_getItemPos(const CFL_ARRAYP array, void *item);

/**
 * @brief Sets the value of the element at the specified index.
 * @param array Pointer to the array.
 * @param ulIndex Index of the element to set.
 * @param item Pointer to the data to copy into the element.
 */
extern void cfl_array_set(CFL_ARRAYP array, CFL_UINT32 ulIndex,
                          const void *item);

/**
 * @brief Clears all elements from the array.
 * @param array Pointer to the array.
 * @note This resets the length to zero but does not free memory.
 */
extern void cfl_array_clear(CFL_ARRAYP array);

/**
 * @brief Returns the current number of elements in the array.
 * @param array Pointer to the array.
 * @return Number of elements in the array.
 */
extern CFL_UINT32 cfl_array_length(CFL_ARRAYP array);

/**
 * @brief Sets the length of the array.
 * @param array Pointer to the array.
 * @param newLen New length for the array.
 * @note If newLen is greater than current length, new elements are zeroed.
 */
extern void cfl_array_setLength(CFL_ARRAYP array, CFL_UINT32 newLen);

/**
 * @brief Creates a copy of an array.
 * @param other Pointer to the array to clone.
 * @return Pointer to the new cloned array, or NULL if allocation fails.
 */
extern CFL_ARRAYP cfl_array_clone(const CFL_ARRAYP other);

/**
 * @brief Creates an iterator for traversing the array.
 * @param array Pointer to the array.
 * @return Iterator for the array.
 */
extern CFL_ITERATORP cfl_array_iterator(CFL_ARRAYP array);

#ifdef __cplusplus
}
#endif

#endif
