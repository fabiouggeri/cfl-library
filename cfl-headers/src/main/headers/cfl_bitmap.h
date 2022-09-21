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

#ifndef CFL_BITMAP_H_

#define CFL_BITMAP_H_

#include "cfl_types.h"

struct _CFL_BITMAP {
   CFL_UINT8  *map;
   CFL_UINT16 uiSize;
};

extern CFL_BITMAPP cfl_bitmap_new(CFL_UINT16 numBits);
extern void cfl_bitmap_free(CFL_BITMAPP pBitMap);
extern void cfl_bitmap_set(CFL_BITMAPP pBitMap, CFL_UINT16 uiPos) ;
extern void cfl_bitmap_reset(CFL_BITMAPP pBitMap, CFL_UINT16 uiPos);
extern CFL_UINT8 cfl_bitmap_get(CFL_BITMAPP bitMap, CFL_UINT16 uiPos);
extern void cfl_bitmap_clear(CFL_BITMAPP pBitMap);
extern CFL_BOOL cfl_bitmap_equals(CFL_BITMAPP pBitMap1, CFL_BITMAPP pBitMap2);
extern CFL_BOOL cfl_bitmap_isSubSet(CFL_BITMAPP pBitMap1, CFL_BITMAPP pBitMap2);
extern CFL_BITMAPP cfl_bitmap_clone(CFL_BITMAPP pBitMap);

#endif
