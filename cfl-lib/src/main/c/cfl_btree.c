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

#include <stdlib.h>
#include <string.h>

#include "cfl_btree.h"
#include "cfl_iterator.h"
#include "cfl_mem.h"

typedef struct _BTreeIterator {
   CFL_ITERATOR    iterator;
   CFL_INT32       lKey;
   CFL_BTREE_NODEP pNode;
   struct _BTreeIterator *pPreviousIt;
} BTreeIterator;

static CFL_BOOL cfl_btree_iterator_hasNext(CFL_ITERATORP pIt);
static void *cfl_btree_iterator_next(CFL_ITERATORP pIt);
static void *cfl_btree_iterator_value(CFL_ITERATORP pIt);
static void cfl_btree_iterator_free(CFL_ITERATORP pIt);
static void cfl_btree_iterator_first(CFL_ITERATORP pIt);
static void cfl_btree_iterator_last(CFL_ITERATORP pIt);
static CFL_BOOL cfl_btree_iterator_hasPrevious(CFL_ITERATORP pIt);
static void *cfl_btree_iterator_previous(CFL_ITERATORP pIt);

static CFL_ITERATOR_CLASS cfl_btree_iterator_class = {
   cfl_btree_iterator_hasNext,
   cfl_btree_iterator_next,
   cfl_btree_iterator_value,
   NULL,
   cfl_btree_iterator_free,
   cfl_btree_iterator_first,
   cfl_btree_iterator_hasPrevious,
   cfl_btree_iterator_previous,
   cfl_btree_iterator_last,
   NULL,
};

static CFL_BTREE_NODEP cfl_btree_node_new(CFL_BTREEP pTree) {
   CFL_BTREE_NODEP pNode = (CFL_BTREE_NODEP) CFL_MEM_ALLOC(sizeof(CFL_BTREE_NODE) + (sizeof(void *) * pTree->lKeys * 2));
   pNode->pTree = pTree;
   pNode->lNumKeys = 0;
   pNode->bIsLeafNode = CFL_TRUE;
   memset(pNode->pPointers, 0, sizeof(void *) * ((pTree->lKeys * 2) + 1));
   return pNode;
}

static void cfl_btree_node_free(CFL_BTREE_NODEP pNode) {
   CFL_MEM_FREE(pNode);
}

static CFL_INT16 cfl_btree_compareValues(CFL_BTREEP pTree, void * pValue1, void * pValue2, CFL_BOOL bExact) {
   CFL_INT16 iValue = -1;
   if (pTree->pCompareValues != NULL) {
      iValue = pTree->pCompareValues(pValue1, pValue2, bExact);
   }
   return iValue;
}

static CFL_INT32 cfl_btree_node_search(CFL_BTREE_NODEP pNode, void * pKey, CFL_BOOL bExact) {
   CFL_INT32 lLeftIndex = 0;
   CFL_INT32 lRightIndex = pNode->lNumKeys - 1;

   while (lLeftIndex <= lRightIndex) {
      CFL_INT32 lMiddleIndex = lLeftIndex + ((lRightIndex - lLeftIndex) / 2);
      CFL_INT16 iCmp = cfl_btree_compareValues(pNode->pTree, GET_KEY(pNode, lMiddleIndex), pKey, bExact);
      if (iCmp < 0) {
         lLeftIndex = lMiddleIndex + 1;
      } else if (iCmp > 0) {
         lRightIndex = lMiddleIndex - 1;
      } else {
         return lMiddleIndex;
      }
   }

   return -1;
}

static CFL_INT32 cfl_btree_node_keyAscPosition(CFL_BTREE_NODEP pNode, void * pKey) {
   if (pNode->lNumKeys > 1) {
      CFL_INT32 lMiddle;
      CFL_INT32 lFirst = 0;
      CFL_INT32 lLast = pNode->lNumKeys - 1;

      while (lLast - lFirst >= 1) {
         lMiddle = (lLast + lFirst) / 2;

         // Executa a funcao de comparacao
         if (cfl_btree_compareValues(pNode->pTree, pKey, GET_KEY(pNode, lMiddle), CFL_TRUE) <= 0) {
            // Pesquisar do meio pra tras
            lLast = lMiddle;
         } else {
            // Pesquisar do meio pra frente
            lFirst = lMiddle + 1;
         }
      }
      return lLast;
   }
   return 0;
}

static CFL_INT32 cfl_btree_node_keyDescPosition(CFL_BTREE_NODEP pNode, void * pKey) {
   if (pNode->lNumKeys > 1) {
      CFL_INT32 lMiddle;
      CFL_INT32 lFirst = 0;
      CFL_INT32 lLast = pNode->lNumKeys - 1;

      while (lLast - lFirst >= 1) {
         lMiddle = (lLast + lFirst + 1) / 2;

         // Executa a funcao de comparacao
         if (cfl_btree_compareValues(pNode->pTree, pKey, GET_KEY(pNode, lMiddle), CFL_TRUE) >= 0) {
            // Pesquisar do meio pra frente
            lFirst = lMiddle;
         } else {
            // Pesquisar do meio pra tras
            lLast = lMiddle - 1;
         }
      }
      return lFirst;
   }
   return 0;
}

// Remove an element from a node and also the left (0) or right (+1) child.

static void * cfl_btree_node_remove(CFL_BTREE_NODEP pNode, CFL_INT32 lIndex, CFL_INT32 lLeftOrRightChild) {
   if (lIndex >= 0) {
      CFL_INT32 i;
      void * pKeyDeleted;
      for (i = lIndex; i < pNode->lNumKeys - 1; i++) {
         SET_KEY(pNode, i, GET_KEY(pNode, i + 1));
         if (!pNode->bIsLeafNode) {
            if (i >= lIndex + lLeftOrRightChild) {
               SET_CHILD(pNode, i, GET_CHILD(pNode, i + 1));
            }
         }
      }
      pKeyDeleted = GET_KEY(pNode, i);
      SET_KEY(pNode, i, NULL);
      if (!pNode->bIsLeafNode) {
         if (i >= lIndex + lLeftOrRightChild) {
            SET_CHILD(pNode, i, GET_CHILD(pNode, i + 1));
         }
         cfl_btree_node_free(GET_CHILD(pNode, i + 1));
         SET_CHILD(pNode, i + 1, NULL);
      }
      --pNode->lNumKeys;
      return pKeyDeleted;
   }
   return NULL;
}

static void cfl_btree_node_shiftRightByOne(CFL_BTREE_NODEP pNode) {
   CFL_INT32 i;
   if (!pNode->bIsLeafNode) {
      SET_CHILD(pNode, pNode->lNumKeys + 1, GET_CHILD(pNode, pNode->lNumKeys));
   }
   for (i = pNode->lNumKeys - 1; i >= 0; i--) {
      SET_KEY(pNode, i + 1, GET_KEY(pNode, i));
      if (!pNode->bIsLeafNode) {
         SET_CHILD(pNode, i + 1, GET_CHILD(pNode, i));
      }
   }
}

static CFL_INT32 cfl_btree_node_subtreeRootNodeIndex(CFL_BTREE_NODEP pNode, void * pKey) {
   CFL_INT32 i = cfl_btree_node_keyAscPosition(pNode, pKey);
   if (cfl_btree_compareValues(pNode->pTree, pKey, GET_KEY(pNode, i), CFL_TRUE) < 0) {
      return i;
   }
   return pNode->lNumKeys;
}

CFL_BTREEP cfl_btree_new(CFL_INT32 lKeys, BTREE_CMP_VALUE_FUNC pCompareValues) {
   CFL_BTREEP pTree;
   pTree = (CFL_BTREEP) CFL_MEM_ALLOC(sizeof(CFL_BTREE));
   pTree->lKeys = lKeys;
   pTree->pCompareValues = pCompareValues;
   pTree->pRoot = cfl_btree_node_new(pTree);
   return pTree;
}

static void cfl_btree_freeNodes(CFL_BTREE_NODEP pNode, BTREE_FREE_KEY_FUNC pFreeKey) {
   if (!pNode->bIsLeafNode) {
      CFL_INT32 i;
      if (pFreeKey != NULL) {
         for (i = 0; i < pNode->lNumKeys; i++) {
            pFreeKey(GET_KEY(pNode, i));
            cfl_btree_freeNodes(GET_CHILD(pNode, i), pFreeKey);
         }
         cfl_btree_freeNodes(GET_CHILD(pNode, i), pFreeKey);
      } else {
         for (i = 0; i <= pNode->lNumKeys; i++) {
            cfl_btree_freeNodes(GET_CHILD(pNode, i), pFreeKey);
         }
      }
   }
   cfl_btree_node_free(pNode);
}

void cfl_btree_free(CFL_BTREEP pTree, BTREE_FREE_KEY_FUNC pFreeKey) {
   cfl_btree_freeNodes(pTree->pRoot, pFreeKey);
   CFL_MEM_FREE(pTree);
}

// Split the node, node, of a B-Tree into two nodes that both contain T-1 elements and move node's median key up to the pParentNode.
// This method will only be called if node is full; node is the i-th child of pParentNode.

static void cfl_btree_splitChildNode(CFL_BTREE_NODEP pParentNode, CFL_INT32 i, CFL_BTREE_NODEP pNode) {
   CFL_BTREEP pTree = pNode->pTree;
   CFL_BTREE_NODEP pNewNode = cfl_btree_node_new(pTree);
   CFL_INT32 lMiddle = (pTree->lKeys + 1) / 2;
   CFL_INT32 j;
   pNewNode->bIsLeafNode = pNode->bIsLeafNode;
   pNewNode->lNumKeys = lMiddle - 1;
   for (j = 0; j < pNewNode->lNumKeys; j++) { // Copy the last T-1 elements of pNode into pNewNode.
      SET_KEY(pNewNode, j, GET_KEY(pNode, j + lMiddle));
      SET_KEY(pNode, j + lMiddle, NULL);
   }
   if (!pNewNode->bIsLeafNode) {
      for (j = 0; j < lMiddle; j++) { // Copy the last T pointers of pNode into pNewNode.
         SET_CHILD(pNewNode, j, GET_CHILD(pNode, j + lMiddle));
      }
      for (j = lMiddle; j <= pNode->lNumKeys; j++) {
         cfl_btree_node_free(GET_CHILD(pNode, j));
         SET_CHILD(pNode, j, NULL);
      }
   }
   //   for (j = lMiddle; j < pNode->lNumKeys; j++) {
   //      SET_KEY(pNode, j, NULL);
   //   }
   pNode->lNumKeys = lMiddle - 1;

   // Insert a (child) pointer to node pNewNode into the pParentNode, moving other keys and pointers as necessary.
   for (j = pParentNode->lNumKeys; j >= i + 1; j--) {
      SET_CHILD(pParentNode, j + 1, GET_CHILD(pParentNode, j));
   }
   SET_CHILD(pParentNode, i + 1, pNewNode);
   for (j = pParentNode->lNumKeys - 1; j >= i; j--) {
      SET_KEY(pParentNode, j + 1, GET_KEY(pParentNode, j));
   }
   SET_KEY(pParentNode, i, GET_KEY(pNode, lMiddle - 1));
   SET_KEY(pNode, lMiddle - 1, NULL);
   ++(pParentNode->lNumKeys);
}

// Insert an element into a B-Tree. (The element will ultimately be inserted into a leaf pNode).

static void cfl_btree_insertIntoNonFullNode(CFL_BTREE_NODEP pNode, void * pKey) {
   CFL_BTREEP pTree = pNode->pTree;
   CFL_INT32 i = pNode->lNumKeys - 1;
   if (pNode->bIsLeafNode) {
      // Since node is not a full node insert the new element into its proper place within node.
      while (i >= 0 && cfl_btree_compareValues(pTree, pKey, GET_KEY(pNode, i), CFL_TRUE) < 0) {
         SET_KEY(pNode, i + 1, GET_KEY(pNode, i));
         --i;
      }
      ++i;
      SET_KEY(pNode, i, pKey);
      ++(pNode->lNumKeys);
   } else {
      // Move back from the last key of node until we find the child pointer to the node
      // that is the root node of the subtree where the new element should be placed.
      while (i >= 0 && cfl_btree_compareValues(pTree, pKey, GET_KEY(pNode, i), CFL_TRUE) < 0) {
         --i;
      }
      ++i;
      if (GET_CHILD(pNode, i)->lNumKeys == pTree->lKeys) {
         cfl_btree_splitChildNode(pNode, i, GET_CHILD(pNode, i));
         if (cfl_btree_compareValues(pTree, pKey, GET_KEY(pNode, i), CFL_TRUE) > 0) {
            ++i;
         }
      }
      cfl_btree_insertIntoNonFullNode(GET_CHILD(pNode, i), pKey);
   }
}

static CFL_BOOL cfl_btree_existsKey(CFL_BTREE_NODEP pNode, void * pKey) {
   while (pNode != NULL) {
      CFL_INT32 i = cfl_btree_node_keyAscPosition(pNode, pKey);
      if (cfl_btree_compareValues(pNode->pTree, pKey, GET_KEY(pNode, i), CFL_TRUE) == 0) {
         return CFL_TRUE;
      } else {
         ++i;
      }
      if (pNode->bIsLeafNode) {
         return CFL_FALSE;
      } else {
         pNode = GET_CHILD(pNode, i);
      }
   }
   return CFL_FALSE;
}

CFL_BOOL cfl_btree_add(CFL_BTREEP pTree, void * pKey) {
   CFL_BTREE_NODEP pRootNode = pTree->pRoot;
   if (! cfl_btree_existsKey(pTree->pRoot, pKey)) {
      if (pRootNode->lNumKeys == pTree->lKeys) {
         CFL_BTREE_NODEP pNewRootNode = cfl_btree_node_new(pTree);
         pTree->pRoot = pNewRootNode;
         pNewRootNode->bIsLeafNode = CFL_FALSE;
         SET_CHILD(pNewRootNode, 0, pRootNode);
         cfl_btree_splitChildNode(pNewRootNode, 0, pRootNode); // Split pRootNode and move its median (middle) key up into pNewRootNode.
         cfl_btree_insertIntoNonFullNode(pNewRootNode, pKey); // Insert the key into the B-Tree with root pNewRootNode.
      } else {
         cfl_btree_insertIntoNonFullNode(pRootNode, pKey); // Insert the key into the B-Tree with root pRootNode.
      }
      return CFL_TRUE;
   }
   return CFL_FALSE;
}

// Merge two nodes and keep the median key (element) empty.

static CFL_INT32 cfl_btree_mergeNodes(CFL_BTREE_NODEP pDstNode, CFL_BTREE_NODEP pSrcNode) {
   CFL_INT32 lMedianKeyIndex;
   if (cfl_btree_compareValues(pSrcNode->pTree, GET_KEY(pSrcNode, 0), GET_KEY(pDstNode, pDstNode->lNumKeys - 1), CFL_TRUE) < 0) {
      CFL_INT32 i;
      // Shift all elements of pDstNode right by pSrcNode.mNumKeys + 1 to make place for the pSrcNode and the median key.
      if (!pDstNode->bIsLeafNode) {
         SET_CHILD(pDstNode, pSrcNode->lNumKeys + pDstNode->lNumKeys + 1, GET_CHILD(pDstNode, pDstNode->lNumKeys));
      }
      for (i = pDstNode->lNumKeys; i > 0; i--) {
         SET_KEY(pDstNode, pSrcNode->lNumKeys + i, GET_KEY(pDstNode, i - 1));
         if (!pDstNode->bIsLeafNode) {
            SET_CHILD(pDstNode, pSrcNode->lNumKeys + i, GET_CHILD(pDstNode, i - 1));
         }
      }

      // Clear the median key (element).
      lMedianKeyIndex = pSrcNode->lNumKeys;
      SET_KEY(pDstNode, lMedianKeyIndex, NULL);

      // Copy the pSrcNode's elements into pDstNode.
      for (i = 0; i < pSrcNode->lNumKeys; i++) {
         SET_KEY(pDstNode, i, GET_KEY(pSrcNode, i));
         if (!pSrcNode->bIsLeafNode) {
            SET_CHILD(pDstNode, i, GET_CHILD(pSrcNode, i));
         }
      }
      if (!pSrcNode->bIsLeafNode) {
         SET_CHILD(pDstNode, i, GET_CHILD(pSrcNode, i));
      }
   } else {
      CFL_INT32 lOffset;
      CFL_INT32 i;

      // Clear the median key (element).
      lMedianKeyIndex = pDstNode->lNumKeys;
      SET_KEY(pDstNode, lMedianKeyIndex, NULL);

      // Copy the pSrcNode's elements into pDstNode.
      lOffset = lMedianKeyIndex + 1;
      for (i = 0; i < pSrcNode->lNumKeys; i++) {
         SET_KEY(pDstNode, lOffset + i, GET_KEY(pSrcNode, i));
         if (!pSrcNode->bIsLeafNode) {
            SET_CHILD(pDstNode, lOffset + i, GET_CHILD(pSrcNode, i));
         }
      }
      if (!pSrcNode->bIsLeafNode) {
         SET_CHILD(pDstNode, lOffset + i, GET_CHILD(pSrcNode, i));
      }
   }
   pDstNode->lNumKeys += pSrcNode->lNumKeys;
   return lMedianKeyIndex;
}

// Move the key from srcNode at index into dstNode at medianKeyIndex. Note that the element at index is already empty.

static void cfl_btree_moveKey(CFL_BTREE_NODEP pSrcNode, CFL_INT32 lSrcKeyIndex, CFL_INT32 lChildIndex, CFL_BTREE_NODEP pDstNode, CFL_INT32 lMedianKeyIndex) {
   SET_KEY(pDstNode, lMedianKeyIndex, GET_KEY(pSrcNode, lSrcKeyIndex));
   ++(pDstNode->lNumKeys);

   cfl_btree_node_remove(pSrcNode, lSrcKeyIndex, lChildIndex);

   if (pSrcNode == pDstNode->pTree->pRoot && pSrcNode->lNumKeys == 0) {
      pDstNode->pTree->pRoot = pDstNode;
   }
}

static void * cfl_btree_deleteFromNode(CFL_BTREE_NODEP pNode, void * pKey) {
   // 1. If the key is in node and node is a leaf node, then delete the key from node.
   void * pDeletedKey = NULL;
   if (pNode->bIsLeafNode) {
      CFL_INT32 i;
      if ((i = cfl_btree_node_search(pNode, pKey, CFL_TRUE)) != -1) { // key is i-th key of node if node contains key.
         pDeletedKey = cfl_btree_node_remove(pNode, i, LEFT_CHILD_NODE);
      }
   } else {
      CFL_INT32 i;
      CFL_INT32 lMiddle = (pNode->pTree->lKeys + 1) / 2;
      // 2. If node is an internal node and it contains the key... (key is i-th key of node if node contains key)
      if ((i = cfl_btree_node_search(pNode, pKey, CFL_TRUE)) != -1) {
         CFL_BTREE_NODEP pLeftChildNode = GET_CHILD(pNode, i);
         CFL_BTREE_NODEP pRightChildNode = GET_CHILD(pNode, i + 1);
         // 2a. If the predecessor child node has at least T keys...
         if (pLeftChildNode->lNumKeys >= lMiddle) {
            CFL_BTREE_NODEP pPredecessorNode = pLeftChildNode;
            CFL_BTREE_NODEP pErasureNode = pPredecessorNode; // Make sure not to delete a key from a node with only T - 1 elements.
            while (!pPredecessorNode->bIsLeafNode) { // Therefore only descend to the previous node (erasureNode) of the predecessor node and delete the key using 3.
               pErasureNode = pPredecessorNode;
               pPredecessorNode = GET_CHILD(pPredecessorNode, pNode->lNumKeys - 1);
            }
            SET_KEY(pNode, i, GET_KEY(pPredecessorNode, pPredecessorNode->lNumKeys - 1));
            pDeletedKey = cfl_btree_deleteFromNode(pErasureNode, GET_KEY(pNode, i));

            // 2b. If the successor child node has at least T keys...
         } else if (pRightChildNode->lNumKeys >= lMiddle) {
            CFL_BTREE_NODEP pSuccessorNode = pRightChildNode;
            CFL_BTREE_NODEP pErasureNode = pSuccessorNode; // Make sure not to delete a key from a node with only T - 1 elements.
            while (!pSuccessorNode->bIsLeafNode) { // Therefore only descend to the previous node (erasureNode) of the predecessor node and delete the key using 3.
               pErasureNode = pSuccessorNode;
               pSuccessorNode = GET_CHILD(pSuccessorNode, 0);
            }
            SET_KEY(pNode, i, GET_KEY(pSuccessorNode, 0));
            pDeletedKey = cfl_btree_deleteFromNode(pErasureNode, GET_KEY(pNode, i));

            // 2c. If both the predecessor and the successor child node have only T - 1 keys...
            // If both of the two child nodes to the left and right of the deleted element have the minimum number of elements,
            // namely T - 1, they can then be joined into a single node with 2 * T - 2 elements.
         } else {
            CFL_INT32 lMedianKeyIndex = cfl_btree_mergeNodes(pLeftChildNode, pRightChildNode);
            cfl_btree_node_free(pRightChildNode);
            cfl_btree_moveKey(pNode, i, RIGHT_CHILD_NODE, pLeftChildNode, lMedianKeyIndex); // Delete i's right child pointer from node.
            pDeletedKey = cfl_btree_deleteFromNode(pLeftChildNode, pKey);
         }

         // 3. If the key is not present in node, descent to the root of the appropriate subtree that must contain key...
         // The method is structured to guarantee that whenever delete is called recursively on node "node", the number of keys
         // in node is at least the minimum degree T. Note that this condition requires one more key than the minimum required
         // by usual B-tree conditions. This strengthened condition allows us to delete a key from the tree in one downward pass
         // without having to "back up".
      } else {
         CFL_BTREE_NODEP pChildNode;
         i = cfl_btree_node_subtreeRootNodeIndex(pNode, pKey);
         pChildNode = GET_CHILD(pNode, i); // childNode is i-th child of node.
         if (pChildNode->lNumKeys == lMiddle - 1) {
            CFL_BTREE_NODEP pLeftChildSibling = (i - 1 >= 0) ? GET_CHILD(pNode, i - 1) : NULL;
            CFL_BTREE_NODEP pRightChildSibling = (i + 1 <= pNode->lNumKeys) ? GET_CHILD(pNode, i + 1) : NULL;
            // 3a. The left sibling has >= T keys...
            if (pLeftChildSibling != NULL && pLeftChildSibling->lNumKeys >= lMiddle) {
               // Move a key from the subtree's root node down into childNode along with the appropriate child pointer.
               // Therefore, first shift all elements and children of childNode right by 1.
               cfl_btree_node_shiftRightByOne(pChildNode);
               SET_KEY(pChildNode, 0, GET_KEY(pNode, i - 1)); // i - 1 is the key index in node that is smaller than childNode's smallest key.
               if (!pChildNode->bIsLeafNode) {
                  SET_CHILD(pChildNode, 0, GET_CHILD(pLeftChildSibling, pLeftChildSibling->lNumKeys));
               }
               ++(pChildNode->lNumKeys);

               // Move a key from the left sibling into the subtree's root node.
               SET_KEY(pNode, i - 1, GET_KEY(pLeftChildSibling, pLeftChildSibling->lNumKeys - 1));

               // Remove the key from the left sibling along with its right child node.
               cfl_btree_node_remove(pLeftChildSibling, pLeftChildSibling->lNumKeys - 1, RIGHT_CHILD_NODE);

               // 3b. The right sibling has >= T keys...
            } else if (pRightChildSibling != NULL && pRightChildSibling->lNumKeys >= lMiddle) {
               // Move a key from the subtree's root node down into childNode along with the appropriate child pointer.
               SET_KEY(pChildNode, pChildNode->lNumKeys, GET_KEY(pNode, i)); // i is the key index in node that is bigger than childNode's biggest key.
               if (!pChildNode->bIsLeafNode) {
                  SET_CHILD(pChildNode, pChildNode->lNumKeys + 1, GET_CHILD(pRightChildSibling, 0));
               }
               ++(pChildNode->lNumKeys);

               // Move a key from the right sibling into the subtree's root node.
               SET_KEY(pNode, i, GET_KEY(pRightChildSibling, 0));
               // Remove the key from the right sibling along with its left child node.
               cfl_btree_node_remove(pRightChildSibling, 0, LEFT_CHILD_NODE);

               // 3c. Both of childNode's siblings have only T - 1 keys each...
            } else {
               if (pLeftChildSibling != NULL) {
                  CFL_INT32 lMedianKeyIndex = cfl_btree_mergeNodes(pChildNode, pLeftChildSibling);
                  cfl_btree_node_free(pLeftChildSibling);
                  cfl_btree_moveKey(pNode, i - 1, LEFT_CHILD_NODE, pChildNode, lMedianKeyIndex); // i - 1 is the median key index in node when merging with the left sibling.
               } else if (pRightChildSibling != NULL) {
                  CFL_INT32 lMedianKeyIndex = cfl_btree_mergeNodes(pChildNode, pRightChildSibling);
                  cfl_btree_node_free(pRightChildSibling);
                  cfl_btree_moveKey(pNode, i, RIGHT_CHILD_NODE, pChildNode, lMedianKeyIndex); // i is the median key index in node when merging with the right sibling.
               }
            }
         }
         pDeletedKey = cfl_btree_deleteFromNode(pChildNode, pKey);
      }
   }
   return pDeletedKey;
}

void * cfl_btree_delete(CFL_BTREEP pTree, void * pKey) {
   return cfl_btree_deleteFromNode(pTree->pRoot, pKey);
}

// Iterative search method.

static void * cfl_btree_searchFromNode(CFL_BTREE_NODEP pNode, void * pKey) {
   while (pNode != NULL) {
      CFL_INT32 i = cfl_btree_node_keyAscPosition(pNode, pKey);
      if (cfl_btree_compareValues(pNode->pTree, pKey, GET_KEY(pNode, i), CFL_TRUE) == 0) {
         return GET_KEY(pNode, i);
      } else {
         ++i;
      }
      if (pNode->bIsLeafNode) {
         return NULL;
      } else {
         pNode = GET_CHILD(pNode, i);
      }
   }
   return NULL;
}

void * cfl_btree_search(CFL_BTREEP pTree, void * pKey) {
   return cfl_btree_searchFromNode(pTree->pRoot, pKey);
}

// search node by position

static void * cfl_btree_searchPositionFromNode(CFL_BTREE_NODEP pNode, CFL_INT32 *lCount) {
   CFL_INT32 i;
   if (pNode->bIsLeafNode) {
      if (*lCount <= pNode->lNumKeys) {
         i = *lCount - 1;
         *lCount = 0;
         return GET_KEY(pNode, i);
      } else {
         *lCount -= pNode->lNumKeys;
      }
   } else {
      for (i = 0; i < pNode->lNumKeys; i++) {
         void * pFoundKey = cfl_btree_searchPositionFromNode(GET_CHILD(pNode, i), lCount);
         if (pFoundKey != NULL) {
            return pFoundKey;
         } else {
            *lCount -= 1;
            if (*lCount == 0) {
               return GET_KEY(pNode, i);
            }
         }
      }
   }
   return NULL;
}

void * cfl_btree_searchPosition(CFL_BTREEP pTree, CFL_INT32 lPosition) {
   return cfl_btree_searchPositionFromNode(pTree->pRoot, &lPosition);
}

// Iterative search method.

static void * cfl_btree_searchLikeFromNode(CFL_BTREE_NODEP pNode, void * pKey) {
   void * pFoundKey = NULL;
   CFL_INT32 i = cfl_btree_node_keyAscPosition(pNode, pKey);
   if (cfl_btree_compareValues(pNode->pTree, pKey, GET_KEY(pNode, i), CFL_TRUE) == 0) {
      return GET_KEY(pNode, i);
   } else {
      ++i;
   }
   if (!pNode->bIsLeafNode) {
      pFoundKey = cfl_btree_searchLikeFromNode(GET_CHILD(pNode, i), pKey);
      /* Se nao encontrou a chave exata E
         E a chave pesquisada eh menor que a chave corrente do noh corrente (nao extrapolou a ultima chave)
         E a chave eh parcialmente igual a chave corrente do noh corrente,
         ENTAO retorna essa chave */
      if (pFoundKey == NULL && i < pNode->lNumKeys && cfl_btree_compareValues(pNode->pTree, pKey, GET_KEY(pNode, i), CFL_FALSE) == 0) {
         pFoundKey = GET_KEY(pNode, i);
      }
   }
   return pFoundKey;
}

void * cfl_btree_searchLike(CFL_BTREEP pTree, void * pKey) {
   return cfl_btree_searchLikeFromNode(pTree->pRoot, pKey);
}

static BTreeIterator *cfl_btree_iteratorCreate(CFL_BTREE_NODEP pNode, CFL_INT32 lKey, BTreeIterator *pPreviousIt) {
   BTreeIterator *pIt = (BTreeIterator *) CFL_MEM_ALLOC(sizeof(BTreeIterator));
   pIt->iterator.itClass = &cfl_btree_iterator_class;
   pIt->pNode = pNode;
   pIt->lKey = lKey;
   pIt->pPreviousIt = pPreviousIt;
   return pIt;
}

static CFL_ITERATORP cfl_btree_iteratorSearchFromNode(CFL_BTREE_NODEP pNode, void * pKey) {
   BTreeIterator *pParentIt = NULL;
   while (pNode != NULL) {
      CFL_INT32 i = cfl_btree_node_keyAscPosition(pNode, pKey);
      if (cfl_btree_compareValues(pNode->pTree, pKey, GET_KEY(pNode, i), CFL_TRUE) == 0) {
         BTreeIterator *pIt = cfl_btree_iteratorCreate(pNode, i, pParentIt);
         return &pIt->iterator;
      } else {
         ++i;
      }
      if (pNode->bIsLeafNode) {
         pNode = NULL;
      } else {
         pNode = GET_CHILD(pNode, i);
         pParentIt = cfl_btree_iteratorCreate(pNode, i, NULL);
      }
   }
   if (pParentIt != NULL) {
      cfl_btree_iterator_free(&pParentIt->iterator);
   }
   return NULL;
}

CFL_ITERATORP cfl_btree_iteratorSearch(CFL_BTREEP pTree, void * pKey) {
   return cfl_btree_iteratorSearchFromNode(pTree->pRoot, pKey);
}

static BTreeIterator *cfl_btree_iteratorSearchLikeFromNode(CFL_BTREE_NODEP pNode, void * pKey, BTreeIterator *pParentIt) {
   CFL_INT32 i = cfl_btree_node_keyAscPosition(pNode, pKey);
   if (cfl_btree_compareValues(pNode->pTree, pKey, GET_KEY(pNode, i), CFL_TRUE) == 0) {
      return cfl_btree_iteratorCreate(pNode, i, pParentIt);
   } else {
      ++i;
   }
   if (!pNode->bIsLeafNode) {
      BTreeIterator *pIterator = cfl_btree_iteratorSearchLikeFromNode(GET_CHILD(pNode, i), pKey, cfl_btree_iteratorCreate(pNode, i, NULL));
      /* Se nao encontrou a chave exata E
         E a chave pesquisada eh menor que a chave corrente do noh corrente (nao extrapolou a ultima chave)
         E a chave eh parcialmente igual a chave corrente do noh corrente,
         ENTAO retorna essa chave */
      if (pIterator == NULL && i < pNode->lNumKeys && cfl_btree_compareValues(pNode->pTree, pKey, GET_KEY(pNode, i), CFL_FALSE) == 0) {
         return cfl_btree_iteratorCreate(pNode, i, pParentIt);
      }
   }
   /* Se nao encontrar a chave libera o iterator pai antes de retornar */
   if (pParentIt != NULL) {
      CFL_MEM_FREE(pParentIt);
   }
   return NULL;
}

CFL_ITERATORP cfl_btree_iteratorSearchLike(CFL_BTREEP pTree, void * pKey) {
   BTreeIterator *pIterator = cfl_btree_iteratorSearchLikeFromNode(pTree->pRoot, pKey, NULL);
   if (pIterator != NULL) {
      return &pIterator->iterator;
   }
   return NULL;
}

static BTreeIterator *cfl_btree_iteratorSoftSearchLikeFromNode(CFL_BTREE_NODEP pNode, void * pKey, BTreeIterator *pParentIt) {
   CFL_INT32 i = cfl_btree_node_keyAscPosition(pNode, pKey);
   if (cfl_btree_compareValues(pNode->pTree, pKey, GET_KEY(pNode, i), CFL_TRUE) == 0) {
      BTreeIterator *pIterator = cfl_btree_iteratorCreate(pNode, i, pParentIt);
      return pIterator;
   } else {
      ++i;
   }
   if (!pNode->bIsLeafNode) {
      BTreeIterator *pIterator = cfl_btree_iteratorSoftSearchLikeFromNode(GET_CHILD(pNode, i), pKey, cfl_btree_iteratorCreate(pNode, i, NULL));
      /* Se nao encontrou a chave exata E
         E a chave pesquisada eh menor que a chave corrente do noh corrente (nao extrapolou a ultima chave)
         E a chave eh parcialmente igual a chave corrente do noh corrente,
         ENTAO retorna essa chave */
      if (pIterator == NULL && i < pNode->lNumKeys && cfl_btree_compareValues(pNode->pTree, pKey, GET_KEY(pNode, i), CFL_FALSE) >= 0) {
         return cfl_btree_iteratorCreate(pNode, i, pParentIt);
      }
   }
   /* Se nao encontrar a chave libera o iterator pai antes de retornar */
   if (pParentIt != NULL) {
      CFL_MEM_FREE(pParentIt);
   }
   return NULL;
}

CFL_ITERATORP cfl_btree_iteratorSoftSearchLike(CFL_BTREEP pTree, void * pKey) {
   BTreeIterator *pIterator = cfl_btree_iteratorSoftSearchLikeFromNode(pTree->pRoot, pKey, NULL);
   if (pIterator != NULL) {
      return &pIterator->iterator;
   }
   return NULL;
}

static BTreeIterator *cfl_btree_iteratorSearchLastLikeFromNode(CFL_BTREE_NODEP pNode, void * pKey, BTreeIterator *pParentIt) {
   CFL_INT32 i = cfl_btree_node_keyDescPosition(pNode, pKey);
   if (cfl_btree_compareValues(pNode->pTree, pKey, GET_KEY(pNode, i), CFL_TRUE) == 0) {
      return cfl_btree_iteratorCreate(pNode, i, pParentIt);
   } else {
      ++i;
   }
   if (!pNode->bIsLeafNode) {
      BTreeIterator *pIterator = cfl_btree_iteratorSearchLastLikeFromNode(GET_CHILD(pNode, i), pKey, cfl_btree_iteratorCreate(pNode, i, NULL));
      /* Se nao encontrou a chave exata E
         E a chave pesquisada eh menor que a chave corrente do noh corrente (nao extrapolou a ultima chave)
         E a chave eh parcialmente igual a chave corrente do noh corrente,
         ENTAO retorna essa chave */
      if (pIterator == NULL && i < pNode->lNumKeys && cfl_btree_compareValues(pNode->pTree, pKey, GET_KEY(pNode, i), CFL_FALSE) == 0) {
         return cfl_btree_iteratorCreate(pNode, i, pParentIt);
      }
   }
   /* Se nao encontrar a chave libera o iterator pai antes de retornar */
   if (pParentIt != NULL) {
      CFL_MEM_FREE(pParentIt);
   }
   return NULL;
}

CFL_ITERATORP cfl_btree_iteratorSearchLastLike(CFL_BTREEP pTree, void * pKey) {
   BTreeIterator *pIterator = cfl_btree_iteratorSearchLastLikeFromNode(pTree->pRoot, pKey, NULL);
   if (pIterator != NULL) {
      return &pIterator->iterator;
   }
   return NULL;
}

static BTreeIterator *cfl_btree_iteratorSoftSearchLastLikeFromNode(CFL_BTREE_NODEP pNode, void * pKey, BTreeIterator *pParentIt) {
   CFL_INT32 i = cfl_btree_node_keyDescPosition(pNode, pKey);
   if (cfl_btree_compareValues(pNode->pTree, pKey, GET_KEY(pNode, i), CFL_TRUE) == 0) {
      return cfl_btree_iteratorCreate(pNode, i, pParentIt);
   } else {
      ++i;
   }
   if (!pNode->bIsLeafNode) {
      BTreeIterator *pIterator = cfl_btree_iteratorSoftSearchLastLikeFromNode(GET_CHILD(pNode, i), pKey, cfl_btree_iteratorCreate(pNode, i, NULL));
      /* Se nao encontrou a chave exata E
         E a chave pesquisada eh menor que a chave corrente do noh corrente (nao extrapolou a ultima chave)
         E a chave eh parcialmente igual a chave corrente do noh corrente,
         ENTAO retorna essa chave */
      if (pIterator == NULL && i < pNode->lNumKeys && cfl_btree_compareValues(pNode->pTree, pKey, GET_KEY(pNode, i), CFL_FALSE) <= 0) {
         return cfl_btree_iteratorCreate(pNode, i, pParentIt);
      }
   }
   /* Se nao encontrar a chave libera o iterator pai antes de retornar */
   if (pParentIt != NULL) {
      CFL_MEM_FREE(pParentIt);
   }
   return NULL;
}

CFL_ITERATORP cfl_btree_iteratorSoftSearchLastLike(CFL_BTREEP pTree, void * pKey) {
   BTreeIterator *pIterator = cfl_btree_iteratorSoftSearchLastLikeFromNode(pTree->pRoot, pKey, NULL);
   if (pIterator != NULL) {
      return &pIterator->iterator;
   }
   return NULL;
}

CFL_ITERATORP cfl_btree_iterator(CFL_BTREEP pTree) {
   BTreeIterator *pIt = NULL;
   CFL_BTREE_NODEP pNode = pTree->pRoot;
   /* Busca o no mais a esquerda da arvore, que deve ser o de menor chave... */
   while (!pNode->bIsLeafNode) {
      pIt = cfl_btree_iteratorCreate(pNode, 0, pIt);
      pNode = GET_CHILD(pNode, 0);
   }
   pIt = cfl_btree_iteratorCreate(pNode, 0, pIt);
   return &pIt->iterator;
}

CFL_ITERATORP cfl_btree_iteratorLast(CFL_BTREEP pTree) {
   BTreeIterator *pIt = NULL;
   CFL_BTREE_NODEP pNode = pTree->pRoot;
   /* Busca o no mais a direita da arvore, que deve ser o de menor chave... */
   while (!pNode->bIsLeafNode) {
      pIt = cfl_btree_iteratorCreate(pNode, pNode->lNumKeys - 1, pIt);
      pNode = GET_CHILD(pNode, pNode->lNumKeys);
   }
   pIt = cfl_btree_iteratorCreate(pNode, pNode->lNumKeys - 1, pIt);
   return &pIt->iterator;
}

static void cfl_btree_iterator_free(CFL_ITERATORP iterator) {
   BTreeIterator *pIt = (BTreeIterator *) iterator;
   while (pIt != NULL) {
      BTreeIterator *pPrevIt = pIt->pPreviousIt;
      CFL_MEM_FREE(pIt);
      pIt = pPrevIt;
   }
}

static void cfl_btree_iterator_first(CFL_ITERATORP iterator) {
   BTreeIterator *pIt = (BTreeIterator *) iterator;
   BTreeIterator *pPreviousIt;
   CFL_BTREE_NODEP pNode = pIt->pNode->pTree->pRoot;

   pPreviousIt = pIt->pPreviousIt;
   while (pPreviousIt != NULL) {
      BTreeIterator *pAuxIt = pPreviousIt->pPreviousIt;
      CFL_MEM_FREE(pPreviousIt);
      pPreviousIt = pAuxIt;
   }
   pPreviousIt = NULL;
   /* Busca o no mais a esquerda da arvore, que deve ser o de menor chave... */
   while (!pNode->bIsLeafNode) {
      pPreviousIt = cfl_btree_iteratorCreate(pNode, 0, pPreviousIt);
      pNode = GET_CHILD(pNode, 0);
   }
   pIt->pNode = pNode;
   pIt->lKey = 0;
   pIt->pPreviousIt = pPreviousIt;
}

static void cfl_btree_iterator_last(CFL_ITERATORP iterator) {
   BTreeIterator *pIt = (BTreeIterator *) iterator;
   BTreeIterator *pPreviousIt;
   CFL_BTREE_NODEP pNode = pIt->pNode->pTree->pRoot;

   pPreviousIt = pIt->pPreviousIt;
   while (pPreviousIt != NULL) {
      BTreeIterator *pAuxIt = pPreviousIt->pPreviousIt;
      CFL_MEM_FREE(pPreviousIt);
      pPreviousIt = pAuxIt;
   }
   pPreviousIt = NULL;
   /* Busca o no mais a esquerda da arvore, que deve ser o de menor chave... */
   while (!pNode->bIsLeafNode) {
      pPreviousIt = cfl_btree_iteratorCreate(pNode, pNode->lNumKeys - 1, pPreviousIt);
      pNode = GET_CHILD(pNode, pNode->lNumKeys);
   }
   pIt->pNode = pNode;
   pIt->lKey = pNode->lNumKeys - 1;
   pIt->pPreviousIt = pPreviousIt;
}

static CFL_BOOL cfl_btree_iterator_hasNext(CFL_ITERATORP iterator) {
   BTreeIterator *pIt = (BTreeIterator *) iterator;

   while (pIt != NULL) {
      if (pIt->lKey < pIt->pNode->lNumKeys) {
         return CFL_TRUE;
      } else {
         pIt = pIt->pPreviousIt;
      }
   }
   return CFL_FALSE;
}

static CFL_BOOL cfl_btree_iterator_hasPrevious(CFL_ITERATORP iterator) {
   BTreeIterator *pIt = (BTreeIterator *) iterator;

   while (pIt != NULL) {
      if (pIt->lKey > 0) {
         return CFL_TRUE;
      } else {
         pIt = pIt->pPreviousIt;
      }
   }
   return CFL_FALSE;
}

static void * cfl_btree_iterator_next(CFL_ITERATORP iterator) {
   BTreeIterator *pIt = (BTreeIterator *) iterator;
   CFL_BOOL bGoingback = CFL_FALSE;

   while (CFL_TRUE) {
      if (pIt->lKey < pIt->pNode->lNumKeys) {
         if (pIt->pNode->bIsLeafNode || bGoingback) {
            return GET_KEY(pIt->pNode, (pIt->lKey)++);
         } else {
            CFL_BTREE_NODEP pChildNode = GET_CHILD(pIt->pNode, pIt->lKey);
            BTreeIterator *pParentIt = (BTreeIterator *) CFL_MEM_ALLOC(sizeof(BTreeIterator));
            memcpy(pParentIt, pIt, sizeof (BTreeIterator));
            pIt->pNode = pChildNode;
            pIt->lKey = 0;
            pIt->pPreviousIt = pParentIt;
            continue;
         }
      } else if (pIt->pPreviousIt != NULL) {
         BTreeIterator *pAuxIt = pIt->pPreviousIt;
         pIt->pNode = pAuxIt->pNode;
         pIt->lKey = pAuxIt->lKey;
         pIt->pPreviousIt = pAuxIt->pPreviousIt;
         CFL_MEM_FREE(pAuxIt);
         bGoingback = CFL_TRUE;
         continue;
      }
      return NULL;
   }
}

static void * cfl_btree_iterator_value(CFL_ITERATORP iterator) {
   BTreeIterator *pIt = (BTreeIterator *) iterator;
   if (pIt != NULL && pIt->pNode != NULL && pIt->lKey >= 0) {
      return GET_KEY(pIt->pNode, pIt->lKey);
   }
   return NULL;
}

static void * cfl_btree_iterator_previous(CFL_ITERATORP iterator) {
   BTreeIterator *pIt = (BTreeIterator *) iterator;
   CFL_BOOL bGoingback = CFL_FALSE;

   while (CFL_TRUE) {
      if (pIt->lKey > 0) {
         if (pIt->pNode->bIsLeafNode || bGoingback) {
            return GET_KEY(pIt->pNode, --(pIt->lKey));
         } else {
            CFL_BTREE_NODEP pChildNode = GET_CHILD(pIt->pNode, pIt->lKey);
            BTreeIterator *pParentIt = (BTreeIterator *) CFL_MEM_ALLOC(sizeof(BTreeIterator));
            memcpy(pParentIt, pIt, sizeof(BTreeIterator));
            pIt->pNode = pChildNode;
            pIt->lKey = pChildNode->lNumKeys;
            pIt->pPreviousIt = pParentIt;
            continue;
         }
      } else if (pIt->pPreviousIt != NULL) {
         BTreeIterator *pAuxIt = pIt->pPreviousIt;
         pIt->pNode = pAuxIt->pNode;
         pIt->lKey = pAuxIt->lKey;
         pIt->pPreviousIt = pAuxIt->pPreviousIt;
         CFL_MEM_FREE(pAuxIt);
         bGoingback = CFL_TRUE;
         continue;
      }
   }
}

CFL_BOOL cfl_btree_walk(CFL_BTREE_NODEP pNode, BTREE_WALK_CALLBACK callback) {
   int i;
   if (callback(pNode)) {
      for (i = 0; i < pNode->lNumKeys; i++) {
         if (! cfl_btree_walk(GET_CHILD(pNode, i), callback)) {
            return CFL_FALSE;
         }
      }
      return CFL_TRUE;
   }
   return CFL_FALSE;
}

