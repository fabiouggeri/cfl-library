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

#ifndef CFL_NUMBER_H_

#define CFL_NUMBER_H_

#include "cfl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// #define CLF_NUM(name, size) uint32_t name[sizeof(int)+sizeof(size_t)+sizeof(size_t)+sizeof(size_t)+size]

typedef struct _CFL_NUM_BITS {
    CFL_UINT16  numBits;      // number of valid bits
    CFL_UINT16  wordCapacity; // capaity in words
    CFL_UINT32 *words;        // 32 bits words, LSB-first
} CFL_NUM_BITS, *CFL_NUM_BITSP;


typedef struct _CFL_NUMBER {
    int          sign;       // -1, 0, +1
    CFL_UINT16   scale;      // decimal digits after the decimal point
    CFL_NUM_BITS magnitude;  // non-negative magnitude (bits)
} CFL_NUMBER, *CFL_NUMBERP;

extern void cfl_number_init(CFL_NUMBER *x);
extern void cfl_number_free(CFL_NUMBER *x);
extern char *cfl_number_to_string(const CFL_NUMBER *x);
extern CFL_NUMBER cfl_number_from_string(const char *s);
extern CFL_NUMBER cfl_number_from_int64(CFL_INT64 v);
extern int cfl_number_div(const CFL_NUMBER *A, const CFL_NUMBER *B, CFL_UINT16 out_scale, CFL_NUMBER *Out);
extern CFL_NUMBER cfl_number_mul(const CFL_NUMBER *A, const CFL_NUMBER *B);
extern CFL_NUMBER cfl_number_sub(const CFL_NUMBER *A, const CFL_NUMBER *B);
extern CFL_NUMBER cfl_number_add(const CFL_NUMBER *A, const CFL_NUMBER *B);
extern CFL_NUMBER cfl_number_from_double(double val, CFL_UINT16 scale);
extern double cfl_number_to_double(const CFL_NUMBER *n);

#ifdef __cplusplus
}
#endif

#endif
