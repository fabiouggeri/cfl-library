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

#include "cfl_atomic.h"

#if defined(__BORLANDC__)
   #define DEFINE_OPERATIONS_SET(datatype, typename) \
      datatype cfl_atomic_set##typename(VOLATILE_PARAM datatype *var, datatype value) {\
         datatype previousValue = *var;\
         *var = value;\
         return previousValue;\
      }\
      datatype cfl_atomic_compareAndSet##typename(VOLATILE_PARAM datatype *var, datatype oldValue, datatype newValue) {\
         datatype previousValue = *var;\
         if (previousValue == oldValue) {\
            *var = newValue;\
         }\
         return previousValue;\
      }

   #define DEFINE_OPERATIONS_OP(datatype, typename) \
      datatype cfl_atomic_add##typename(VOLATILE_PARAM datatype *var, datatype value) {\
         datatype previousValue = *var;\
         *var = *var + value;\
         return previousValue;\
      }\
      datatype cfl_atomic_sub##typename(VOLATILE_PARAM datatype *var, datatype value) {\
         datatype previousValue = *var;\
         *var = *var - value;\
         return previousValue;\
      }\
      datatype cfl_atomic_and##typename(VOLATILE_PARAM datatype *var, datatype value) {\
         datatype previousValue = *var;\
         *var = *var & value;\
         return previousValue;\
      }\
      datatype cfl_atomic_or##typename(VOLATILE_PARAM datatype *var, datatype value) {\
         datatype previousValue = *var;\
         *var = *var | value;\
         return previousValue;\
      }\
      datatype cfl_atomic_xor##typename(VOLATILE_PARAM datatype *var, datatype value) {\
         datatype previousValue = *var;\
         *var = *var ^ value;\
         return previousValue;\
      }

      DEFINE_OPERATIONS_SET(CFL_BOOL , Boolean)

      DEFINE_OPERATIONS_SET(CFL_INT8 , Int8   )
      DEFINE_OPERATIONS_OP(CFL_INT8 , Int8   )

      DEFINE_OPERATIONS_SET(CFL_INT16, Int16  )
      DEFINE_OPERATIONS_OP(CFL_INT16, Int16  )

      DEFINE_OPERATIONS_SET(CFL_INT32, Int32  )
      DEFINE_OPERATIONS_OP(CFL_INT32, Int32  )

      DEFINE_OPERATIONS_SET(CFL_INT64, Int64  )
      DEFINE_OPERATIONS_OP(CFL_INT64, Int64  )

      DEFINE_OPERATIONS_SET(void *   , Pointer)

#elif defined(CFL_OS_WINDOWS) && ! defined(__GNUC__)
   #include <intrin.h>

   #define DEFINE_OPERATIONS_SET(datatype, typename, nativetype, datasize) \
      datatype cfl_atomic_set##typename(VOLATILE_PARAM datatype *var, datatype value) {\
         return _InterlockedExchange##datasize((nativetype *)var, (nativetype) value);\
      }\
      datatype cfl_atomic_compareAndSet##typename(VOLATILE_PARAM datatype *var, datatype oldValue, datatype newValue) {\
         return _InterlockedCompareExchange##datasize((nativetype *)var, (nativetype) newValue, (nativetype) oldValue);\
      }

   #define DEFINE_OPERATIONS_OP(datatype, typename, nativetype, datasize) \
      datatype cfl_atomic_add##typename(VOLATILE_PARAM datatype *var, datatype value) {\
         return _InterlockedExchangeAdd##datasize((nativetype *)var, (nativetype) value);\
      }\
      datatype cfl_atomic_sub##typename(VOLATILE_PARAM datatype *var, datatype value) {\
         return _InterlockedExchangeAdd##datasize((nativetype *)var, (nativetype) (-1 * value));\
      }\
      datatype cfl_atomic_and##typename(VOLATILE_PARAM datatype *var, datatype value) {\
         return _InterlockedAnd##datasize((nativetype *)var, (nativetype) value);\
      }\
      datatype cfl_atomic_or##typename(VOLATILE_PARAM datatype *var, datatype value) {\
         return _InterlockedOr##datasize((nativetype *)var, (nativetype) value);\
      }\
      datatype cfl_atomic_xor##typename(VOLATILE_PARAM datatype *var, datatype value) {\
         return _InterlockedXor##datasize((nativetype *)var, (nativetype) value);\
      }

   DEFINE_OPERATIONS_SET(CFL_BOOL , Boolean, char  , 8 )

   DEFINE_OPERATIONS_SET(CFL_INT8 , Int8   , char  , 8 )
   DEFINE_OPERATIONS_OP(CFL_INT8 , Int8   , char  , 8 )

   DEFINE_OPERATIONS_SET(CFL_INT16, Int16  , short , 16)
   DEFINE_OPERATIONS_OP(CFL_INT16, Int16  , short , 16)

   DEFINE_OPERATIONS_SET(CFL_INT32, Int32  , long  ,   )
   DEFINE_OPERATIONS_OP(CFL_INT32, Int32  , long  ,   )

#if defined(CFL_ARCH_64)
   DEFINE_OPERATIONS_SET(void *   , Pointer, void *, Pointer )
   DEFINE_OPERATIONS_SET(CFL_INT64, Int64  , __int64, 64)
   DEFINE_OPERATIONS_OP(CFL_INT64, Int64  , __int64, 64)
#endif

#else

   #define DEFINE_OPERATIONS_SET(datatype, typename) \
      datatype cfl_atomic_set##typename(VOLATILE_PARAM datatype *var, datatype value) {\
         return __sync_lock_test_and_set(var, value);\
      }\
      datatype cfl_atomic_compareAndSet##typename(VOLATILE_PARAM datatype *var, datatype oldValue, datatype newValue) {\
         return __sync_val_compare_and_swap(var, oldValue, newValue);\
      }

   #define DEFINE_OPERATIONS_OP(datatype, typename) \
      datatype cfl_atomic_add##typename(VOLATILE_PARAM datatype *var, datatype value) {\
         return __sync_fetch_and_add(var, value);\
      }\
      datatype cfl_atomic_sub##typename(VOLATILE_PARAM datatype *var, datatype value) {\
         return __sync_fetch_and_sub(var, value);\
      }\
      datatype cfl_atomic_and##typename(VOLATILE_PARAM datatype *var, datatype value) {\
         return __sync_fetch_and_and(var, value);\
      }\
      datatype cfl_atomic_or##typename(VOLATILE_PARAM datatype *var, datatype value) {\
         return __sync_fetch_and_or(var, value);\
      }\
      datatype cfl_atomic_xor##typename(VOLATILE_PARAM datatype *var, datatype value) {\
         return __sync_fetch_and_xor(var, value);\
      }

   DEFINE_OPERATIONS_SET(CFL_BOOL , Boolean)

   DEFINE_OPERATIONS_SET(CFL_INT8 , Int8   )
   DEFINE_OPERATIONS_OP(CFL_INT8 , Int8   )

   DEFINE_OPERATIONS_SET(CFL_INT16, Int16  )
   DEFINE_OPERATIONS_OP(CFL_INT16, Int16  )

   DEFINE_OPERATIONS_SET(CFL_INT32, Int32  )
   DEFINE_OPERATIONS_OP(CFL_INT32, Int32  )

   DEFINE_OPERATIONS_SET(CFL_INT64, Int64  )
   DEFINE_OPERATIONS_OP(CFL_INT64, Int64  )

   DEFINE_OPERATIONS_SET(void *   , Pointer)

#endif

