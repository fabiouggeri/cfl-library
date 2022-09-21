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

#ifndef _CFL_BTREE_H_

#define _CFL_BTREE_H_

#include "cfl_types.h"

#define LEFT_CHILD_NODE 0
#define RIGHT_CHILD_NODE 1

#if defined( __cplusplus ) && ! defined( __IBMCPP__ )
   extern "C" {
#endif

#define GET_KEY(t,i)     (t->pPointers[(i*2)+1])
#define GET_CHILD(t,i)   ((CFL_BTREE_NODEP)t->pPointers[i*2])
#define SET_KEY(t,i,p)   (t->pPointers[(i*2)+1]=(void *)p)
#define SET_CHILD(t,i,p) (t->pPointers[i*2]=(void *)p)

struct _CFL_BTREE_NODE {
  CFL_BTREEP  pTree;
  CFL_INT32   lNumKeys;
  CFL_BOOL    bIsLeafNode;
  void *      pPointers[1];
};

struct _CFL_BTREE {
   CFL_BTREE_NODEP      pRoot;
   BTREE_CMP_VALUE_FUNC pCompareValues;
   CFL_INT32            lKeys;
};

extern CFL_BTREEP cfl_btree_new(CFL_INT32 lKeys, BTREE_CMP_VALUE_FUNC pCompareValues);
extern void cfl_btree_free(CFL_BTREEP pTree, BTREE_FREE_KEY_FUNC pFreeKey);
extern CFL_BOOL cfl_btree_add(CFL_BTREEP pTree, void *pKey);
extern void *cfl_btree_delete(CFL_BTREEP pTree, void *pKey);
extern void *cfl_btree_search(CFL_BTREEP pTree, void *pKey);
extern void *cfl_btree_searchPosition(CFL_BTREEP pTree, CFL_INT32 lPosition);
extern void *cfl_btree_searchLike(CFL_BTREEP pTree, void *pKey);
extern CFL_ITERATORP cfl_btree_iteratorSearch(CFL_BTREEP pTree, void *pKey);
extern CFL_ITERATORP cfl_btree_iteratorSearchLike(CFL_BTREEP pTree, void *pKey);
extern CFL_ITERATORP cfl_btree_iteratorSoftSearchLike(CFL_BTREEP pTree, void *pKey);
extern CFL_ITERATORP cfl_btree_iteratorSearchLastLike(CFL_BTREEP pTree, void *pKey);
extern CFL_ITERATORP cfl_btree_iteratorSoftSearchLastLike(CFL_BTREEP pTree, void *pKey);
extern CFL_ITERATORP cfl_btree_iterator(CFL_BTREEP pTree);
extern CFL_ITERATORP cfl_btree_iteratorLast(CFL_BTREEP pTree);
extern CFL_BOOL cfl_btree_walk(CFL_BTREE_NODEP pNode, BTREE_WALK_CALLBACK callback);

#if defined( __cplusplus ) && ! defined( __IBMCPP__ )
   }
#endif

#endif