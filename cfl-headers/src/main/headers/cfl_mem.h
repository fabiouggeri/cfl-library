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

#ifndef _CFL_MEM_H_

#define _CFL_MEM_H_

#include <stdlib.h>

#include "cfl_types.h"

typedef void* (*CFL_MALLOC_FUNC)(CFL_UINT64);
typedef void* (*CFL_REALLOC_FUNC)(void *, CFL_UINT64);
typedef void (*CFL_FREE_FUNC)(void *);

#define CFL_MEM_ALLOC(s)      cfl_malloc(s)
#define CFL_MEM_REALLOC(m,s)  cfl_realloc(m,s)
#define CFL_MEM_FREE(m)       cfl_free(m)

extern void cfl_mem_set(CFL_MALLOC_FUNC malloc_func, CFL_REALLOC_FUNC realloc_func, CFL_FREE_FUNC free_func);
extern void * cfl_malloc(CFL_UINT64 size);
extern void * cfl_realloc(void *ptr, CFL_UINT64 size);
extern void cfl_free(void *ptr);

#endif
