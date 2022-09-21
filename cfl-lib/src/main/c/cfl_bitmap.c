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
#include <string.h>
#include "cfl_bitmap.h"

CFL_BITMAPP cfl_bitmap_new(CFL_UINT16 uiNumBits) {
   CFL_BITMAPP bitMap = (CFL_BITMAPP) malloc(sizeof(CFL_BITMAP));
   bitMap->uiSize = (uiNumBits / 8) + 1;
   bitMap->map = (CFL_UINT8 *) malloc(bitMap->uiSize);
   memset(bitMap->map, 0, bitMap->uiSize);
   return bitMap;
}

void cfl_bitmap_free(CFL_BITMAPP bitMap) {
   if (bitMap != NULL) {
      free(bitMap->map);
      free(bitMap);
   }
}

void cfl_bitmap_set(CFL_BITMAPP bitMap, CFL_UINT16 uiPos) {
   CFL_UINT8 *pWord;
   CFL_UINT16 uiWordPos = (uiPos / 8);
   if (uiWordPos < bitMap->uiSize) {
      pWord = bitMap->map + uiWordPos;
      *pWord |= (0x01 << (uiPos % 8) );
   }
}

void cfl_bitmap_togle(CFL_BITMAPP bitMap, CFL_UINT16 uiPos) {
   CFL_UINT8 *pWord;
   CFL_UINT16 uiWordPos = (uiPos / 8);
   if (uiWordPos < bitMap->uiSize) {
      pWord = bitMap->map + uiWordPos;
      *pWord ^= (0x01 << (uiPos % 8) );
   }
}

void cfl_bitmap_reset(CFL_BITMAPP bitMap, CFL_UINT16 uiPos) {
   CFL_UINT8 *pWord;
   CFL_UINT16 uiWordPos = (uiPos / 8);
   if (uiWordPos < bitMap->uiSize ) {
      pWord = bitMap->map + uiWordPos;
      *pWord &= ~(0x01 << (uiPos % 8) );
   }
}

CFL_UINT8 cfl_bitmap_get(CFL_BITMAPP bitMap, CFL_UINT16 uiPos) {
   CFL_UINT8 *pWord;
   CFL_UINT8 value = 0;
   CFL_UINT16 uiWordPos = (uiPos / 8);
   if (uiWordPos < bitMap->uiSize) {
      pWord = bitMap->map + uiWordPos;
      value = *pWord & (0x01 << (uiPos % 8) );
   }
   return value;
}

void cfl_bitmap_clear(CFL_BITMAPP bitMap) {
   memset(bitMap->map, 0, bitMap->uiSize);
}

CFL_BITMAPP cfl_bitmap_clone(CFL_BITMAPP bitMap) {
   CFL_BITMAPP pClone = (CFL_BITMAPP) malloc(sizeof(CFL_BITMAP));
   pClone->uiSize = bitMap->uiSize;
   pClone->map = (CFL_UINT8*) malloc(pClone->uiSize);
   memcpy(pClone->map, bitMap->map, pClone->uiSize);
   return pClone;
}

CFL_BOOL cfl_bitmap_isSubSet(CFL_BITMAPP bitMap1, CFL_BITMAPP bitMap2) {
   CFL_UINT8 *pWord1;
   CFL_UINT8 *pWord2;
   CFL_UINT16 uiCount1;
   CFL_UINT16 uiCount2;

   if (bitMap1->uiSize <= bitMap2->uiSize) {
      pWord1 = bitMap1->map;
      pWord2 = bitMap2->map;
      uiCount1 = bitMap1->uiSize;
      uiCount2 = bitMap1->uiSize;
      while (uiCount1 > 0 && uiCount2 > 0) {
         if ((*pWord1 & *pWord2) != *pWord1) {
            return CFL_FALSE;
         }
         ++pWord1;
         ++pWord2;
         --uiCount1;
         --uiCount2;
      }
      return CFL_TRUE;
   }
   return CFL_FALSE;
}

CFL_BOOL cfl_bitmap_equals(CFL_BITMAPP bitMap1, CFL_BITMAPP bitMap2) {
   CFL_UINT8 *pWord1;
   CFL_UINT8 *pWord2;
   CFL_UINT16 uiCount;

   if (bitMap1->uiSize == bitMap2->uiSize) {
      pWord1 = bitMap1->map;
      pWord2 = bitMap2->map;
      uiCount = bitMap1->uiSize;
      while (uiCount > 0) {
         if (*pWord1 != *pWord2) {
            return CFL_FALSE;
         }
         ++pWord1;
         ++pWord2;
         --uiCount;
      }
      return CFL_TRUE;
   }
   return CFL_FALSE;
}
