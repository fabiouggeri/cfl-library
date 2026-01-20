/**
 * @file cfl_list.h
 * @brief Dynamic array of pointers (generic list) implementation.
 *
 * This module provides a dynamic list data structure that stores generic
 * pointers. It supports automatic resizing when elements are added.
 */

#ifndef CFL_LIST_H_

#define CFL_LIST_H_

#include "cfl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Dynamic list structure for storing pointers.
 */
typedef struct _CFL_LIST {
  void **items;        /**< Array of pointers to stored elements */
  CFL_UINT32 length;   /**< Current number of elements in the list */
  CFL_UINT32 capacity; /**< Maximum elements before reallocation */
  CFL_BOOL allocated;  /**< Whether the structure was dynamically allocated */
} CFL_LIST, *CFL_LISTP;

/**
 * @brief Initializes a list structure with the given capacity.
 * @param list Pointer to the list structure to initialize.
 * @param capacity Initial capacity (number of elements).
 */
extern void cfl_list_init(CFL_LISTP list, CFL_UINT32 capacity);

/**
 * @brief Creates a new list with the given capacity.
 * @param capacity Initial capacity (number of elements).
 * @return Pointer to the new list, or NULL if allocation fails.
 */
extern CFL_LISTP cfl_list_new(CFL_UINT32 capacity);

/**
 * @brief Creates a new list with the given length, pre-setting all items to
 * NULL.
 * @param len Initial length (number of elements).
 * @return Pointer to the new list, or NULL if allocation fails.
 */
extern CFL_LISTP cfl_list_newLen(CFL_UINT32 len);

/**
 * @brief Frees the memory used by a list.
 * @param list Pointer to the list to free.
 * @note This does not free the individual items stored in the list.
 */
extern void cfl_list_free(CFL_LISTP list);

/**
 * @brief Adds an item to the end of the list.
 * @param list Pointer to the list.
 * @param item Pointer to the item to add.
 */
extern void cfl_list_add(CFL_LISTP list, void *item);

/**
 * @brief Deletes the item at the specified index.
 * @param list Pointer to the list.
 * @param ulIndex Index of the item to delete.
 * @note This does not free the deleted item.
 */
extern void cfl_list_del(CFL_LISTP list, CFL_UINT32 ulIndex);

/**
 * @brief Deletes the first occurrence of the specified item.
 * @param list Pointer to the list.
 * @param item Pointer to the item to delete.
 */
extern void cfl_list_delItem(CFL_LISTP list, const void *item);

/**
 * @brief Gets the item at the specified index.
 * @param list Pointer to the list.
 * @param ulIndex Index of the item to retrieve.
 * @return Pointer to the item, or NULL if index is out of bounds.
 */
extern void *cfl_list_get(const CFL_LISTP list, CFL_UINT32 ulIndex);

/**
 * @brief Sets the item at the specified index.
 * @param list Pointer to the list.
 * @param ulIndex Index where to set the item.
 * @param item Pointer to the item to set.
 */
extern void cfl_list_set(CFL_LISTP list, CFL_UINT32 ulIndex, void *item);

/**
 * @brief Clears all items from the list.
 * @param list Pointer to the list.
 * @note This resets the length to zero but does not free memory or items.
 */
extern void cfl_list_clear(CFL_LISTP list);

/**
 * @brief Returns the current number of items in the list.
 * @param list Pointer to the list.
 * @return Number of items in the list.
 */
extern CFL_UINT32 cfl_list_length(const CFL_LISTP list);

/**
 * @brief Sets the length of the list.
 * @param list Pointer to the list.
 * @param newLen New length for the list.
 * @note If newLen is greater than current length, new slots are set to NULL.
 */
extern void cfl_list_setLength(CFL_LISTP list, CFL_UINT32 newLen);

/**
 * @brief Creates a shallow copy of a list.
 * @param other Pointer to the list to clone.
 * @return Pointer to the new cloned list, or NULL if allocation fails.
 */
extern CFL_LISTP cfl_list_clone(const CFL_LISTP other);

/**
 * @brief Removes and returns the item at the specified index.
 * @param list Pointer to the list.
 * @param ulIndex Index of the item to remove.
 * @return Pointer to the removed item, or NULL if index is out of bounds.
 */
extern void *cfl_list_remove(CFL_LISTP list, CFL_UINT32 ulIndex);

/**
 * @brief Removes and returns the last item in the list.
 * @param list Pointer to the list.
 * @return Pointer to the removed item, or NULL if list is empty.
 */
extern void *cfl_list_removeLast(CFL_LISTP list);

#ifdef __cplusplus
}
#endif

#endif
