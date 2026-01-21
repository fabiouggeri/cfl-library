/**
 * @file cfl_btree.h
 * @brief B-tree data structure implementation.
 *
 * This module provides a B-tree implementation for efficient sorted storage
 * and retrieval of data. Supports O(log n) search, insert, and delete
 * operations.
 */

#ifndef _CFL_BTREE_H_

#define _CFL_BTREE_H_

#include "cfl_iterator.h"
#include "cfl_types.h"

#define LEFT_CHILD_NODE 0
#define RIGHT_CHILD_NODE 1

#if defined(__cplusplus)
extern "C" {
#endif

#define GET_KEY(t, i) (t->pPointers[((i) * 2) + 1])
#define GET_CHILD(t, i) ((CFL_BTREE_NODEP)t->pPointers[(i) * 2])
#define SET_KEY(t, i, p) (t->pPointers[((i) * 2) + 1] = (void *)p)
#define SET_CHILD(t, i, p) (t->pPointers[(i) * 2] = (void *)p)

struct _CFL_BTREE;
typedef struct _CFL_BTREE CFL_BTREE;
typedef CFL_BTREE *CFL_BTREEP;

struct _CFL_BTREE_NODE;
typedef struct _CFL_BTREE_NODE CFL_BTREE_NODE;
typedef CFL_BTREE_NODE *CFL_BTREE_NODEP;

/** @brief Comparison function for B-tree keys */
typedef CFL_INT16 (*BTREE_CMP_VALUE_FUNC)(void *pValue1, void *pValue2,
                                          CFL_BOOL bExact);
/** @brief Function to free a B-tree key */
typedef void (*BTREE_FREE_KEY_FUNC)(void *pValue);
/** @brief Callback function for B-tree traversal */
typedef CFL_BOOL (*BTREE_WALK_CALLBACK)(void *pValue);

/**
 * @brief B-tree node structure.
 */
struct _CFL_BTREE_NODE {
  CFL_BTREEP pTree;     /**< Pointer to the tree this node belongs to */
  CFL_INT32 lNumKeys;   /**< Number of keys in this node */
  CFL_BOOL bIsLeafNode; /**< Whether this node is a leaf */
  void *pPointers[];    /**< Flexible array for keys and children */
};

/**
 * @brief B-tree structure.
 */
struct _CFL_BTREE {
  CFL_BTREE_NODEP pRoot;               /**< Root node of the tree */
  BTREE_CMP_VALUE_FUNC pCompareValues; /**< Comparison function */
  CFL_INT32 lKeys;                     /**< Maximum keys per node */
};

/**
 * @brief Creates a new B-tree.
 * @param lKeys Maximum number of keys per node (order).
 * @param pCompareValues Function to compare keys.
 * @return Pointer to the new B-tree, or NULL if allocation fails.
 */
extern CFL_BTREEP cfl_btree_new(CFL_INT32 lKeys,
                                BTREE_CMP_VALUE_FUNC pCompareValues);

/**
 * @brief Frees a B-tree and all its nodes.
 * @param pTree Pointer to the B-tree.
 * @param pFreeKey Optional function to free each key (can be NULL).
 */
extern void cfl_btree_free(CFL_BTREEP pTree, BTREE_FREE_KEY_FUNC pFreeKey);

/**
 * @brief Adds a key to the B-tree.
 * @param pTree Pointer to the B-tree.
 * @param pKey Pointer to the key to add.
 * @return CFL_TRUE if added, CFL_FALSE if key already exists.
 */
extern CFL_BOOL cfl_btree_add(CFL_BTREEP pTree, void *pKey);

/**
 * @brief Deletes a key from the B-tree.
 * @param pTree Pointer to the B-tree.
 * @param pKey Pointer to the key to delete.
 * @return Pointer to the deleted key, or NULL if not found.
 */
extern void *cfl_btree_delete(CFL_BTREEP pTree, void *pKey);

/**
 * @brief Searches for an exact key match.
 * @param pTree Pointer to the B-tree.
 * @param pKey Pointer to the key to search for.
 * @return Pointer to the found key, or NULL if not found.
 */
extern void *cfl_btree_search(CFL_BTREEP pTree, void *pKey);

/**
 * @brief Searches for a key by position.
 * @param pTree Pointer to the B-tree.
 * @param lPosition Position (1-indexed).
 * @return Pointer to the key at that position, or NULL if not found.
 */
extern void *cfl_btree_searchPosition(CFL_BTREEP pTree, CFL_INT32 lPosition);

/**
 * @brief Searches for a key using partial matching.
 * @param pTree Pointer to the B-tree.
 * @param pKey Pointer to the key to search for.
 * @return Pointer to a matching key, or NULL if not found.
 */
extern void *cfl_btree_searchLike(CFL_BTREEP pTree, void *pKey);

/**
 * @brief Creates an iterator starting at a specific key.
 * @param pTree Pointer to the B-tree.
 * @param pKey Key to start from.
 * @return Iterator, or NULL if key not found.
 */
extern CFL_ITERATORP cfl_btree_iteratorSearch(CFL_BTREEP pTree, void *pKey);

/**
 * @brief Creates an iterator using partial key matching.
 * @param pTree Pointer to the B-tree.
 * @param pKey Key pattern to match.
 * @return Iterator, or NULL if no match found.
 */
extern CFL_ITERATORP cfl_btree_iteratorSearchLike(CFL_BTREEP pTree, void *pKey);

/**
 * @brief Creates an iterator using soft partial key matching.
 * @param pTree Pointer to the B-tree.
 * @param pKey Key pattern to match.
 * @return Iterator, or NULL if no match found.
 */
extern CFL_ITERATORP cfl_btree_iteratorSoftSearchLike(CFL_BTREEP pTree,
                                                      void *pKey);

/**
 * @brief Creates an iterator starting at the last matching key.
 * @param pTree Pointer to the B-tree.
 * @param pKey Key pattern to match.
 * @return Iterator, or NULL if no match found.
 */
extern CFL_ITERATORP cfl_btree_iteratorSearchLastLike(CFL_BTREEP pTree,
                                                      void *pKey);

/**
 * @brief Creates an iterator using soft matching from the last match.
 * @param pTree Pointer to the B-tree.
 * @param pKey Key pattern to match.
 * @return Iterator, or NULL if no match found.
 */
extern CFL_ITERATORP cfl_btree_iteratorSoftSearchLastLike(CFL_BTREEP pTree,
                                                          void *pKey);

/**
 * @brief Creates an iterator starting from the first element.
 * @param pTree Pointer to the B-tree.
 * @return Iterator for traversing the tree in order.
 */
extern CFL_ITERATORP cfl_btree_iterator(CFL_BTREEP pTree);

/**
 * @brief Creates an iterator starting from the last element.
 * @param pTree Pointer to the B-tree.
 * @return Iterator for traversing the tree in reverse order.
 */
extern CFL_ITERATORP cfl_btree_iteratorLast(CFL_BTREEP pTree);

/**
 * @brief Walks through all keys in the tree.
 * @param pNode Starting node for the walk.
 * @param callback Function called for each key.
 * @return CFL_TRUE if walk completed, CFL_FALSE if stopped by callback.
 */
extern CFL_BOOL cfl_btree_walk(CFL_BTREE_NODEP pNode,
                               BTREE_WALK_CALLBACK callback);

#if defined(__cplusplus)
}
#endif

#endif
