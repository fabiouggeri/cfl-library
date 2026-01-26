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

#ifndef _CFL_TYPES_H_

#define _CFL_TYPES_H_

#include "cfl_os.h"
#include "cfl_ints.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int8_t        CFL_INT8;
typedef uint8_t       CFL_UINT8;
typedef int16_t       CFL_INT16;
typedef uint16_t      CFL_UINT16;
typedef int32_t       CFL_INT32;
typedef uint32_t      CFL_UINT32;
typedef int64_t       CFL_INT64;
typedef uint64_t      CFL_UINT64;
typedef double        CFL_DOUBLE;
typedef float         CFL_FLOAT;
typedef double        CFL_FLOAT64;
typedef float         CFL_FLOAT32;
typedef unsigned char CFL_BOOL;

#define CFL_FALSE 0
#define CFL_TRUE  1

#ifndef NULL
   #define NULL    0
#endif

#define CFL_MAX_INT_FLOAT  16777216

#define CFL_MAX_INT_DOUBLE 9007199254740992

#define CFL_INT8_MAX         0x7f
#define CFL_INT16_MAX        0x7fff
#define CFL_INT32_MAX        0x7fffffff
#define CFL_INT64_MAX        0x7fffffffffffffff
#define CFL_UINT8_MAX        0xffu
#define CFL_UINT16_MAX       0xffffu
#define CFL_UINT32_MAX       0xffffffffu
#define CFL_UINT64_MAX       0xffffffffffffffffu

#define CFL_UNUSED(x) (void)(x)

#if __STDC_VERSION__ >= 199901L
   #define CFL_INLINE inline
#else
   #define CFL_INLINE
#endif

#define CFL_NO_ERROR_TYPE 0
#define CFL_NO_ERROR_CODE 0

#define CFL_WAIT_FOREVER 0xFFFFFFFF

#if (defined(_MSC_VER) && _MSC_VER < 1900)
   #define snprintf _snprintf
#endif


#ifdef __cplusplus
}
#endif

#endif
