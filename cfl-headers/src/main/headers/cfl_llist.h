/**
 * @file cfl_llist.h
 * @brief Doubly linked list implementation with node caching.
 *
 * This module provides a doubly linked list data structure with a node caching
 * mechanism to reduce memory allocations. Supports efficient insertion and
 * removal at both ends.
 */

#ifndef CFL_LLIST_H_

#define CFL_LLIST_H_

#include "cfl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Node structure for the linked list.
 */
typedef struct _CFL_LNODE {
  void *data;                  /**< Pointer to the stored data */
  struct _CFL_LNODE *previous; /**< Pointer to the previous node */
  struct _CFL_LNODE *next;     /**< Pointer to the next node */
} CFL_LNODE, *CFL_LNODEP;

/**
 * @brief Linked list structure with node caching.
 */
typedef struct _CFL_LLIST {
  CFL_LNODEP first;        /**< First node in the chain (includes cache) */
  CFL_LNODEP last;         /**< Last node in the chain (includes cache) */
  CFL_LNODEP head;         /**< First active (used) node */
  CFL_LNODEP tail;         /**< Last active (used) node */
  CFL_UINT32 maxNodeCache; /**< Maximum number of cached nodes */
  CFL_UINT32 nodeCount;    /**< Total number of nodes (active + cached) */
} CFL_LLIST, *CFL_LLISTP;

/**
 * @brief Creates a new linked list with the specified cache size.
 * @param maxNodeCache Maximum number of unused nodes to keep cached.
 * @return Pointer to the new linked list, or NULL if allocation fails.
 */
extern CFL_LLISTP cfl_llist_new(CFL_UINT32 maxNodeCache);

/**
 * @brief Frees the memory used by a linked list.
 * @param list Pointer to the linked list to free.
 * @note This does not free the individual data items stored in the list.
 */
extern void cfl_llist_free(CFL_LLISTP list);

/**
 * @brief Adds an item to the end of the list.
 * @param list Pointer to the linked list.
 * @param item Pointer to the item to add.
 */
extern void cfl_llist_addLast(CFL_LLISTP list, void *item);

/**
 * @brief Adds an item to the beginning of the list.
 * @param list Pointer to the linked list.
 * @param item Pointer to the item to add.
 */
extern void cfl_llist_addFirst(CFL_LLISTP list, void *item);

/**
 * @brief Gets the last item in the list without removing it.
 * @param list Pointer to the linked list.
 * @return Pointer to the last item, or NULL if list is empty.
 */
extern void *cfl_llist_getLast(const CFL_LLISTP list);

/**
 * @brief Gets the first item in the list without removing it.
 * @param list Pointer to the linked list.
 * @return Pointer to the first item, or NULL if list is empty.
 */
extern void *cfl_llist_getFirst(const CFL_LLISTP list);

/**
 * @brief Removes and returns the last item in the list.
 * @param list Pointer to the linked list.
 * @return Pointer to the removed item, or NULL if list is empty.
 */
extern void *cfl_llist_removeLast(CFL_LLISTP list);

/**
 * @brief Removes and returns the first item in the list.
 * @param list Pointer to the linked list.
 * @return Pointer to the removed item, or NULL if list is empty.
 */
extern void *cfl_llist_removeFirst(CFL_LLISTP list);

#ifdef __cplusplus
}
#endif

#endif
