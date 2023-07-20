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

#ifndef CFL_ATOMIC_H_

#define CFL_ATOMIC_H_

#include "cfl_types.h"

#if defined(CFL_OS_WINDOWS) && ! defined(__GNUC__)
   #define VOLATILE_PARAM volatile
#else
   #define VOLATILE_PARAM
#endif

#define DECLARE_OPERATIONS_SET(datatype, typename) \
   extern datatype cfl_atomic_set##typename(VOLATILE_PARAM datatype *var, datatype value);\
   extern datatype cfl_atomic_compareAndSet##typename(VOLATILE_PARAM datatype *var, datatype oldValue, datatype newValue);

#define DECLARE_OPERATIONS_OP(datatype, typename) \
   extern datatype cfl_atomic_add##typename(VOLATILE_PARAM datatype *var, datatype value);\
   extern datatype cfl_atomic_sub##typename(VOLATILE_PARAM datatype *var, datatype value);\
   extern datatype cfl_atomic_and##typename(VOLATILE_PARAM datatype *var, datatype value);\
   extern datatype cfl_atomic_or##typename(VOLATILE_PARAM datatype *var, datatype value);\
   extern datatype cfl_atomic_xor##typename(VOLATILE_PARAM datatype *var, datatype value)

DECLARE_OPERATIONS_SET(CFL_BOOL , Boolean);

DECLARE_OPERATIONS_SET(CFL_INT8 , Int8);
DECLARE_OPERATIONS_OP(CFL_INT8 , Int8);

DECLARE_OPERATIONS_SET(CFL_INT16, Int16);
DECLARE_OPERATIONS_OP(CFL_INT16, Int16);

DECLARE_OPERATIONS_SET(CFL_INT32, Int32);
DECLARE_OPERATIONS_OP(CFL_INT32, Int32);

#if defined(CFL_OS_WINDOWS)
   #if defined(CFL_ARCH_64)
      DECLARE_OPERATIONS_SET(CFL_INT64, Int64);
      DECLARE_OPERATIONS_OP(CFL_INT64, Int64);
   #endif
#else
   DECLARE_OPERATIONS_SET(CFL_INT64, Int64);
   DECLARE_OPERATIONS_OP(CFL_INT64, Int64);
#endif

DECLARE_OPERATIONS_SET(void *, Pointer);

#endif
