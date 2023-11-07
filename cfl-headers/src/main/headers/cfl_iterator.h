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

#ifndef _CFL_ITERATOR_H_

#define _CFL_ITERATOR_H_

#if defined( __cplusplus )
extern "C" {
#endif

#include "cfl_types.h"

struct _CFL_ITERATOR_CLASS {
   CFL_BOOL (*has_next)(CFL_ITERATORP it);
   void *(*next)(CFL_ITERATORP it);
   void *(*current_value)(CFL_ITERATORP it);
   void (*remove)(CFL_ITERATORP it);
   void (*free)(CFL_ITERATORP it);
   void (*first)(CFL_ITERATORP it);
   CFL_BOOL (*has_previous)(CFL_ITERATORP it);
   void *(*previous)(CFL_ITERATORP it);
   void (*last)(CFL_ITERATORP it);
   void (*add)(CFL_ITERATORP it, void *value);
};

struct _CFL_ITERATOR {
   CFL_ITERATOR_CLASS *itClass;
};

extern CFL_ITERATORP cfl_iterator_new(size_t dataSize);
extern void * cfl_iterator_data(CFL_ITERATORP it);
extern void cfl_iterator_free(CFL_ITERATORP it);
extern CFL_BOOL cfl_iterator_hasPrevious(CFL_ITERATORP it);
extern CFL_BOOL cfl_iterator_hasNext(CFL_ITERATORP it);
extern void *cfl_iterator_next(CFL_ITERATORP it);
extern void *cfl_iterator_value(CFL_ITERATORP it);
extern void cfl_iterator_remove(CFL_ITERATORP it);
extern void cfl_iterator_first(CFL_ITERATORP it);
extern void *cfl_iterator_previous(CFL_ITERATORP it);
extern void cfl_iterator_last(CFL_ITERATORP it);
extern void cfl_iterator_add(CFL_ITERATORP it, void *value);

#if defined( __cplusplus )
}
#endif

#endif