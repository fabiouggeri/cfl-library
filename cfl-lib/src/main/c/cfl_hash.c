/*
 * Copyright (c) 2002, Christopher Clark
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * * Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the distribution.
 *
 * * Neither the name of the original author; nor the names of any contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/* Copyright (C) 2004 Christopher Clark <firstname.lastname@cl.cam.ac.uk> */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#include "cfl_hash.h"
#include "cfl_iterator.h"
#include "cfl_mem.h"

#define ROTL32(x,y) (x << y) | (x >> (32 - y))

#define BIG_CONSTANT(x) (x##LLU)

//-----------------------------------------------------------------------------
// Block read - if your platform needs to do endian-swapping or can only
// handle aligned reads, do the conversion here

#define getblock(p, i) (p[i])

//-----------------------------------------------------------------------------
// Finalization mix - force all bits of a hash block to avalanche

/*
Credit for primes table: Aaron Krowne
 http://br.endernet.org/~akrowne/
 http://planetmath.org/encyclopedia/GoodHashTablePrimes.html
 */

typedef struct _HASH_ITERATOR {
   CFL_ITERATOR    iterator;
   CFL_HASHP       hash;
   CFL_HASH_ENTRYP lastEntry;
   CFL_HASH_ENTRYP currEntry;
   CFL_HASH_ENTRYP nextEntry;
   CFL_UINT32      index;
   CFL_UINT32      lastIndex;
   CFL_UINT32      currIndex;
   CFL_UINT32      nextIndex;
} HASH_ITERATOR, *HASH_ITERATORP;

static CFL_BOOL iteratorHasNext(CFL_ITERATORP it);
static void *iteratorNext(CFL_ITERATORP it);
static void * iteratorValue(CFL_ITERATORP it);
static void iteratorRemove(CFL_ITERATORP it);
static void iteratorFree(CFL_ITERATORP it);
static void iteratorFirst(CFL_ITERATORP it);

static const CFL_ITERATOR_CLASS s_hashIteratorClass = {
   iteratorHasNext,
   iteratorNext,
   iteratorValue,
   iteratorRemove,
   iteratorFree,
   iteratorFirst,
   NULL,
   NULL,
   NULL,
   NULL,
};

static const CFL_UINT32 s_primes[] = {
   53, 97, 193, 389,
   769, 1543, 3079, 6151,
   12289, 24593, 49157, 98317,
   196613, 393241, 786433, 1572869,
   3145739, 6291469, 12582917, 25165843,
   50331653, 100663319, 201326611, 402653189,
   805306457, 1610612741
};

const CFL_UINT32 s_primeTableLen = sizeof (s_primes) / sizeof (s_primes[0]);
const float s_maxLoadFactor = 0.65f;

static CFL_UINT32 indexFor(CFL_UINT32 tablelength, CFL_UINT32 hashvalue) {
   return (hashvalue % tablelength);
};

/*****************************************************************************/
CFL_HASHP cfl_hash_new(CFL_UINT32 minSize, HASH_KEY_FUNC hashFunc, HASH_COMP_FUNC equalFunc, HASH_FREE_FUNC freeFunc) {
   CFL_HASHP hash;
   CFL_UINT32 primeIndex;
   CFL_UINT32 size = s_primes[0];

   /* Check requested hashtable isn't too large */
   if (minSize > (1u << 30)) {
      return NULL;
   }
   /* Enforce size as prime */
   for (primeIndex = 0; primeIndex < s_primeTableLen; primeIndex++) {
      if (s_primes[primeIndex] > minSize) {
         size = s_primes[primeIndex];
         break;
      }
   }
   hash = (CFL_HASHP) CFL_MEM_ALLOC(sizeof (CFL_HASH));
   if (NULL == hash) {
      return NULL;
   }
   hash->table = (CFL_HASH_ENTRY **) CFL_MEM_ALLOC(sizeof (CFL_HASH_ENTRYP) * size);
   if (NULL == hash->table) {
      CFL_MEM_FREE(hash);
      return NULL;
   } /*oom*/
   memset(hash->table, 0, size * sizeof (CFL_HASH_ENTRYP));
   hash->tablelength = size;
   hash->primeindex = primeIndex;
   hash->entrycount = 0;
   hash->hashfn = hashFunc;
   hash->eqfn = equalFunc;
   hash->freefn = freeFunc;
   hash->loadlimit = (CFL_UINT32) ceil(size * s_maxLoadFactor);
   return hash;
}

/*****************************************************************************/
CFL_UINT32 cfl_hash_calc(CFL_HASHP hash, void *key) {
   /* Aim to protect against poor hash functions by adding logic here
    * - logic taken from java 1.4 hashtable source */
   CFL_UINT32 i = hash->hashfn(key);
   i += ~(i << 9);
   i ^= ((i >> 14) | (i << 18)); /* >>> */
   i += (i << 4);
   i ^= ((i >> 10) | (i << 22)); /* >>> */
   return i;
}

/*****************************************************************************/
static int hash_expand(CFL_HASHP hash) {
   /* Double the size of the table to accomodate more entries */
   CFL_HASH_ENTRY **newtable;
   CFL_HASH_ENTRYP e;
   CFL_UINT32 newsize, i, index;
   /* Check we're not hitting max capacity */
   if (hash->primeindex == (s_primeTableLen - 1)) {
      return 0;
   }
   newsize = s_primes[++(hash->primeindex)];

   newtable = (CFL_HASH_ENTRY **) CFL_MEM_ALLOC(sizeof (CFL_HASH_ENTRYP) * newsize);
   if (NULL != newtable) {
      memset(newtable, 0, newsize * sizeof (CFL_HASH_ENTRYP));
      /* This algorithm is not 'stable'. ie. it reverses the list
       * when it transfers entries between the tables */
      for (i = 0; i < hash->tablelength; i++) {
         while (NULL != (e = hash->table[i])) {
            hash->table[i] = e->next;
            index = indexFor(newsize, e->hash);
            e->next = newtable[index];
            newtable[index] = e;
         }
      }
      CFL_MEM_FREE(hash->table);
      hash->table = newtable;

      /* Plan B: realloc instead */
   } else {
      newtable = (CFL_HASH_ENTRY **) CFL_MEM_REALLOC(hash->table, newsize * sizeof (CFL_HASH_ENTRYP));
      if (NULL == newtable) {
         (hash->primeindex)--;
         return 0;
      }
      hash->table = newtable;
      memset(newtable[hash->tablelength], 0, newsize - hash->tablelength);
      for (i = 0; i < hash->tablelength; i++) {
         CFL_HASH_ENTRY **pE;
         for (pE = &(newtable[i]), e = *pE; e != NULL; e = *pE) {
            index = indexFor(newsize, e->hash);
            if (index == i) {
               pE = &(e->next);
            } else {
               *pE = e->next;
               e->next = newtable[index];
               newtable[index] = e;
            }
         }
      }
   }
   hash->tablelength = newsize;
   hash->loadlimit = (CFL_UINT32) ceil(newsize * s_maxLoadFactor);
   return -1;
}

/*****************************************************************************/
CFL_UINT32 cfl_hash_count(const CFL_HASHP hash) {
   return hash->entrycount;
}

/*****************************************************************************/
int cfl_hash_insert(CFL_HASHP hash, void *key, void *value) {
   /* This method allows duplicate keys - but they shouldn't be used */
   CFL_UINT32 index;
   CFL_HASH_ENTRYP e;
   if (++(hash->entrycount) > hash->loadlimit) {
      /* Ignore the return value. If expand fails, we should
       * still try cramming just this value into the existing table
       * -- we may not have memory for a larger table, but one more
       * element may be ok. Next time we insert, we'll try expanding again.*/
      hash_expand(hash);
   }
   e = (CFL_HASH_ENTRYP) CFL_MEM_ALLOC(sizeof (CFL_HASH_ENTRY));
   if (NULL == e) {
      --(hash->entrycount);
      return 0;
   }
   e->hash = cfl_hash_calc(hash, key);
   index = indexFor(hash->tablelength, e->hash);
   e->key = key;
   e->value = value;
   e->next = hash->table[index];
   hash->table[index] = e;
   return -1;
}

/*****************************************************************************/
void * cfl_hash_search(CFL_HASHP hash, void *key) {
   CFL_HASH_ENTRYP e;
   CFL_UINT32 hashvalue, index;
   hashvalue = cfl_hash_calc(hash, key);
   index = indexFor(hash->tablelength, hashvalue);
   e = hash->table[index];
   while (NULL != e) {
      /* Check hash value to short circuit heavier comparison */
      if ((hashvalue == e->hash) && (hash->eqfn(key, e->key))) {
         return e->value;
      }
      e = e->next;
   }
   return NULL;
}

/*****************************************************************************/
void * cfl_hash_remove(CFL_HASHP hash, void *key) {
   /* TODO: consider compacting the table when the load factor drops enough,
    *       or provide a 'compact' method. */

   CFL_HASH_ENTRYP e;
   CFL_HASH_ENTRY **pE;
   void *v;
   CFL_UINT32 hashvalue, index;

   hashvalue = cfl_hash_calc(hash, key);
   index = indexFor(hash->tablelength, cfl_hash_calc(hash, key));
   pE = &(hash->table[index]);
   e = *pE;
   while (NULL != e) {
      /* Check hash value to short circuit heavier comparison */
      if ((hashvalue == e->hash) && (hash->eqfn(key, e->key))) {
         *pE = e->next;
         hash->entrycount--;
         v = e->value;
         hash->freefn(e->key, NULL);
         CFL_MEM_FREE(e);
         return v;
      }
      pE = &(e->next);
      e = e->next;
   }
   return NULL;
}

/*****************************************************************************/

/* free */
void cfl_hash_free(CFL_HASHP hash, CFL_BOOL freeValues) {
   CFL_UINT32 i;
   CFL_HASH_ENTRYP e;
   CFL_HASH_ENTRYP f;
   CFL_HASH_ENTRY **table = hash->table;
   if (freeValues) {
      for (i = 0; i < hash->tablelength; i++) {
         e = table[i];
         while (NULL != e) {
            f = e;
            e = e->next;
            hash->freefn(f->key, f->value);
            CFL_MEM_FREE(f);
         }
      }
   } else {
      for (i = 0; i < hash->tablelength; i++) {
         e = table[i];
         while (NULL != e) {
            f = e;
            e = e->next;
            CFL_MEM_FREE(f);
         }
      }
   }
   CFL_MEM_FREE(hash->table);
   CFL_MEM_FREE(hash);
}

/*****************************************************************************/

/* clear */
void cfl_hash_clear(CFL_HASHP hash, CFL_BOOL freeValues) {
   CFL_UINT32 i;
   CFL_HASH_ENTRYP e;
   CFL_HASH_ENTRYP f;
   CFL_HASH_ENTRY **table = hash->table;
   if (freeValues) {
      for (i = 0; i < hash->tablelength; i++) {
         e = table[i];
         table[i] = NULL;
         while (NULL != e) {
            f = e;
            e = e->next;
            hash->freefn(f->key, f->value);
            CFL_MEM_FREE(f);
         }
      }
   } else {
      for (i = 0; i < hash->tablelength; i++) {
         e = table[i];
         table[i] = NULL;
         while (NULL != e) {
            f = e;
            e = e->next;
            CFL_MEM_FREE(f);
         }
      }
   }
   hash->entrycount = 0;
}

static void findNextEntry(HASH_ITERATORP it) {
   it->lastIndex = it->currIndex;
   it->lastEntry = it->currEntry;
   it->currIndex = it->nextIndex;
   it->currEntry = it->nextEntry;
   if (it->nextEntry != NULL) {
      it->nextEntry = it->nextEntry->next;
   }
   while (it->nextEntry == NULL && it->index < it->hash->tablelength) {
      it->nextIndex = it->index;
      it->nextEntry = it->hash->table[(it->index)++];
      if (it->nextEntry != NULL) {
         break;
      }
   }
}

static CFL_BOOL iteratorHasNext(CFL_ITERATORP it) {
   return ((HASH_ITERATORP)it)->nextEntry != NULL;
}

static void *iteratorNext(CFL_ITERATORP it) {
   findNextEntry((HASH_ITERATORP)it);
   return ((HASH_ITERATORP)it)->currEntry->value;
}

static void * iteratorValue(CFL_ITERATORP it) {
   if (((HASH_ITERATORP)it)->currEntry != NULL) {
      return ((HASH_ITERATORP)it)->currEntry->value;
   }
   return NULL;
}

static void iteratorRemove(CFL_ITERATORP it) {
   HASH_ITERATORP itHash = (HASH_ITERATORP)it;
   if (itHash->currEntry != NULL) {
      if (itHash->lastIndex == itHash->currIndex && itHash->lastEntry != NULL) {
         itHash->lastEntry->next = itHash->currEntry->next;
      } else {
         itHash->hash->table[itHash->currIndex] = itHash->currEntry->next;
      }
      itHash->hash->freefn(itHash->currEntry->key, NULL);
      --itHash->hash->entrycount;
      CFL_MEM_FREE(itHash->currEntry);
      itHash->currEntry = NULL;
      itHash->currIndex = 0;
   }
}

static void iteratorFree(CFL_ITERATORP it) {
   CFL_MEM_FREE(it);
}

static void iteratorFirst(CFL_ITERATORP it) {
   ((HASH_ITERATORP)it)->lastIndex = 0;
   ((HASH_ITERATORP)it)->currIndex = 0;
   ((HASH_ITERATORP)it)->nextIndex = 0;
   ((HASH_ITERATORP)it)->index = 0;
   ((HASH_ITERATORP)it)->lastEntry = NULL;
   ((HASH_ITERATORP)it)->currEntry = NULL;
   ((HASH_ITERATORP)it)->nextEntry = NULL;
   findNextEntry(((HASH_ITERATORP)it));
}

CFL_ITERATORP cfl_hash_iterator(CFL_HASHP hash) {
   HASH_ITERATORP pIt = (HASH_ITERATORP) CFL_MEM_ALLOC(sizeof(HASH_ITERATOR));
   if (pIt == NULL) {
      return NULL;
   }
   pIt->iterator.itClass = (CFL_ITERATOR_CLASS *) &s_hashIteratorClass;
   pIt->hash = hash;
   pIt->lastIndex = 0;
   pIt->currIndex = 0;
   pIt->nextIndex = 0;
   pIt->index = 0;
   pIt->lastEntry = NULL;
   pIt->currEntry = NULL;
   pIt->nextEntry = NULL;
   findNextEntry(pIt);
   return (CFL_ITERATORP) pIt;
}

CFL_UINT32 cfl_hash_murmur3(const void * key, CFL_UINT32 len) {
   if (len > 0) {
      const CFL_UINT8 * data = (const CFL_UINT8*) key;
      const int nblocks = len / 4;
      CFL_UINT32 h1 = (CFL_UINT32)(((unsigned char *)key)[0]);
      CFL_UINT32 c1 = 0xcc9e2d51;
      CFL_UINT32 c2 = 0x1b873593;
      CFL_UINT32 k1;
      const CFL_UINT32 * blocks = (const CFL_UINT32 *) (data + nblocks * 4);
      const CFL_UINT8 * tail = (const CFL_UINT8*) (data + nblocks * 4);
      int i;


      for (i = -nblocks; i; i++) {
         k1 = getblock(blocks, i);
         k1 *= c1;
         k1 = ROTL32(k1, 15);
         k1 *= c2;
         h1 ^= k1;
         h1 = ROTL32(h1, 13);
         h1 = h1 * 5 + 0xe6546b64;
      }

      //----------
      // tail


      k1 = 0;
      switch (len & 3) {
         case 3: k1 ^= tail[2] << 16;
         case 2: k1 ^= tail[1] << 8;
         case 1: k1 ^= tail[0];
            k1 *= c1;
            k1 = ROTL32(k1, 15);
            k1 *= c2;
            h1 ^= k1;
      };

      //----------
      // finalization

      h1 ^= len;
      h1 ^= h1 >> 16;
      h1 *= 0x85ebca6b;
      h1 ^= h1 >> 13;
      h1 *= 0xc2b2ae35;
      h1 ^= h1 >> 16;
      return h1;
   }
   return 0;
}
