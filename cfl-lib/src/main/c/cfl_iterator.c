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
#include "cfl_iterator.h"
#include "cfl_mem.h"

CFL_ITERATORP cfl_iterator_new(size_t dataSize) {
   CFL_ITERATORP it = CFL_MEM_ALLOC(sizeof(CFL_ITERATOR) + dataSize);
   return it;
}

void * cfl_iterator_data(CFL_ITERATORP it) {
   return ((char *) it) + sizeof(CFL_ITERATOR);
}

void cfl_iterator_free(CFL_ITERATORP it) {
   if (it->itClass->free == NULL) {
      CFL_MEM_FREE(it);
   } else {
      it->itClass->free(it);
   }
}

CFL_BOOL cfl_iterator_hasNext(CFL_ITERATORP it) {
   return it->itClass->has_next(it);
}

CFL_BOOL cfl_iterator_hasPrevious(CFL_ITERATORP it) {
   return it->itClass->has_previous != NULL && it->itClass->has_previous(it);
}

void * cfl_iterator_next(CFL_ITERATORP it) {
   return it->itClass->next(it);
}

void * cfl_iterator_value(CFL_ITERATORP it) {
   if (it->itClass->current_value != NULL) {
      return it->itClass->current_value(it);
   }
   return NULL;
}

void * cfl_iterator_previous(CFL_ITERATORP it) {
   if (it->itClass->previous != NULL) {
      return it->itClass->previous(it);
   }
   return NULL;
}

void cfl_iterator_remove(CFL_ITERATORP it) {
   if (it->itClass->remove != NULL) {
      it->itClass->remove(it);
   }
}

void cfl_iterator_add(CFL_ITERATORP it, void *value) {
   if (it->itClass->add != NULL) {
      it->itClass->add(it, value);
   }
}

void cfl_iterator_first(CFL_ITERATORP it) {
   if (it->itClass->first != NULL) {
      it->itClass->first(it);
   }
}

void cfl_iterator_last(CFL_ITERATORP it) {
   if (it->itClass->last != NULL) {
      it->itClass->last(it);
   }
}

