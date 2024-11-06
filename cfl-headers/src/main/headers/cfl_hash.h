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

#ifndef CFL_HASH_H_

#define CFL_HASH_H_

#include "cfl_types.h"
#include "cfl_iterator.h"

#ifdef __cplusplus
extern "C" {
#endif

struct _CFL_HASH_ENTRY;
typedef struct _CFL_HASH_ENTRY CFL_HASH_ENTRY;
typedef CFL_HASH_ENTRY         *CFL_HASH_ENTRYP;

typedef CFL_UINT32 (*HASH_KEY_FUNC) (void *k);
typedef int        (*HASH_COMP_FUNC) (void *k1, void *k2);
typedef void       (*HASH_FREE_FUNC) (void *k, void *v);

struct _CFL_HASH_ENTRY {
    void *key;
    void *value;
    CFL_HASH_ENTRYP next;
    CFL_UINT32 hash;
};

typedef struct _CFL_HASH {
   CFL_HASH_ENTRY **table;
   HASH_KEY_FUNC  hashfn;
   HASH_COMP_FUNC eqfn;
   HASH_FREE_FUNC freefn;
   CFL_UINT32     tablelength;
   CFL_UINT32     entrycount;
   CFL_UINT32     loadlimit;
   CFL_UINT32     primeindex;
} CFL_HASH, *CFL_HASHP;

/*****************************************************************************
 * cfl_hash_new

 * @name                    cfl_hash_new
 * @param   minsize         minimum initial size of hashtable
 * @param   hashfunction    function for hashing keys
 * @param   key_eq_fn       function for determining key equality
 * @return                  newly created hashtable or NULL on failure
 */

extern CFL_HASHP cfl_hash_new(CFL_UINT32 minsize, HASH_KEY_FUNC hashf, HASH_COMP_FUNC eqf, HASH_FREE_FUNC freef);

/*****************************************************************************
 * cfl_hash_insert

 * @name        cfl_hash_insert
 * @param   h   the hashtable to insert into
 * @param   k   the key - hashtable claims ownership and will free on removal
 * @param   v   the value - does not claim ownership
 * @return      non-zero for successful insertion
 *
 * This function will cause the table to expand if the insertion would take
 * the ratio of entries to table size over the maximum load factor.
 *
 * This function does not check for repeated insertions with a duplicate key.
 * The value returned when using a duplicate key is undefined -- when
 * the hashtable changes size, the order of retrieval of duplicate key
 * entries is reversed.
 * If in doubt, remove before insert.
 */

extern int cfl_hash_insert(CFL_HASHP h, void *k, void *v);

/*****************************************************************************
 * cfl_hash_search

 * @name        cfl_hash_search
 * @param   h   the hashtable to search
 * @param   k   the key to search for  - does not claim ownership
 * @return      the value associated with the key, or NULL if none found
 */

extern void * cfl_hash_search(CFL_HASHP h, void *k);

/*****************************************************************************
 * cfl_hash_remove

 * @name        cfl_hash_remove
 * @param   h   the hashtable to remove the item from
 * @param   k   the key to search for  - does not claim ownership
 * @return      the value associated with the key, or NULL if none found
 */

extern void * cfl_hash_remove(CFL_HASHP h, void *k);

/*****************************************************************************
 * cfl_hash_count

 * @name        cfl_hash_count
 * @param   h   the hashtable
 * @return      the number of items stored in the hashtable
 */
extern CFL_UINT32 cfl_hash_count(CFL_HASHP h);


/*****************************************************************************
 * cfl_hash_free

 * @name        cfl_hash_free
 * @param   h   the hashtable
 * @param       free_values     whether to call 'free' on the remaining values
 */

extern void cfl_hash_free(CFL_HASHP h, CFL_BOOL free_values);

/*****************************************************************************
 * cfl_hash_clear

 * @name        cfl_hash_clear
 * @param   h   the hashtable
 * @param       free_values     whether to call 'free' on the remaining values
 */

extern void cfl_hash_clear(CFL_HASHP h, CFL_BOOL free_values);

/*****************************************************************************/
CFL_UINT32 cfl_hash_calc(CFL_HASHP h, void *k);

/*****************************************************************************/
CFL_UINT32 cfl_hash_murmur3(const void * key, CFL_UINT32 len);

CFL_ITERATORP cfl_hash_iterator(CFL_HASHP h);

#ifdef __cplusplus
}
#endif

#endif
