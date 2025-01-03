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

#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "cfl_str.h"

#ifndef va_copy
   #define va_copy(dest, src) dest = src
#endif

#define DEFAULT_CAPACITY 16

static CFL_BOOL ensureCapacityForLen(CFL_STRP str, CFL_UINT32 newLen) {
   if (! str->isVarData) {
      const char *curData = str->data;
      str->dataSize = (newLen >> 1) + 1 + newLen;
      str->data = (char *) CFL_MEM_ALLOC(sizeof(char) * str->dataSize);
      if (str->data != NULL) {
         str->isVarData = CFL_TRUE;
         memcpy(str->data, curData, str->length);
         str->data[str->length] = '\0';
      } else {
         str->dataSize = 0;
      }
   } else if (newLen >= str->dataSize) {
      str->dataSize = (str->dataSize >> 1) + 1 + newLen;
      str->data = (char *) CFL_MEM_REALLOC(str->data, str->dataSize);
   }
   return str->data != NULL;
}

void cfl_str_initCapacity(CFL_STRP str, CFL_UINT32 iniCapacity) {
   if (str == NULL) {
      return;
   }
   str->length = 0;
   str->hashValue = 0;
   str->isAllocated = CFL_FALSE;
   if (iniCapacity > 0) {
      str->dataSize = iniCapacity + 1;
      str->data = (char *) CFL_MEM_ALLOC(str->dataSize * sizeof(char));
      if (str->data != NULL) {
         str->data[0] = '\0';
         str->isVarData = CFL_TRUE;
      } else {
         str->data = "";
         str->isVarData = CFL_FALSE;
      }
   } else {
      str->dataSize = 0;
      str->data = "";
      str->isVarData = CFL_FALSE;
   }
}

void cfl_str_init(CFL_STRP str) {
   if (str == NULL) {
      return;
   }
   str->dataSize = 0;
   str->length = 0;
   str->hashValue = 0;
   str->isVarData = CFL_FALSE;
   str->isAllocated = CFL_FALSE;
   str->data = "";
}

void cfl_str_initConstLen(CFL_STRP str, const char *buffer, CFL_UINT32 len) {
   if (str == NULL) {
      return;
   }
   str->hashValue = 0;
   str->isVarData = CFL_FALSE;
   str->isAllocated = CFL_FALSE;
   if (buffer != NULL && len > 0) {
      str->dataSize = (CFL_UINT32) len + 1;
      str->length = (CFL_UINT32) len;
      str->data = (char* ) buffer;
   } else {
      str->dataSize = 0;
      str->length = 0;
      str->data = "";
   }
}

void cfl_str_initConst(CFL_STRP str, const char *buffer) {
   cfl_str_initConstLen(str, buffer, buffer == NULL ? 0 : (CFL_UINT32) strlen(buffer));
}

void cfl_str_initValue(CFL_STRP str, const char *buffer) {
   size_t len;
   if (str == NULL) {
      return;
   }
   len = buffer != NULL ? strlen(buffer) : 0;
   str->length = (CFL_UINT32) len;
   str->hashValue = 0;
   str->isAllocated = CFL_FALSE;
   if (len > 0) {
      str->dataSize = (CFL_UINT32) len + 1;
      str->data = (char *) CFL_MEM_ALLOC(str->dataSize * sizeof(char));
      if (str->data != NULL) {
         memcpy(str->data, (void *) buffer, len * sizeof (char));
         str->isVarData = CFL_TRUE;
         str->data[len] = '\0';
      } else {
         str->isVarData = CFL_FALSE;
         str->dataSize = 0;
         str->data = "";
      }
   } else {
      str->isVarData = CFL_FALSE;
      str->dataSize = 0;
      str->data = "";
   }
}

CFL_STRP cfl_str_new(CFL_UINT32 iniCapacity) {
   CFL_STRP str;
   if (iniCapacity == 0) {
      return cfl_str_newConstLen(NULL, 0);
   }
   str = (CFL_STRP) CFL_MEM_ALLOC(sizeof(CFL_STR));
   if (str == NULL) {
      return NULL;
   }
   str->length = 0;
   str->hashValue = 0;
   str->isAllocated = CFL_TRUE;
   str->isVarData = CFL_TRUE;
   str->dataSize = iniCapacity + 1;
   str->data = (char *) CFL_MEM_ALLOC(str->dataSize * sizeof(char));
   if (str->data != NULL) {
      str->data[0] = '\0';
   } else {
      CFL_MEM_FREE(str);
      str = NULL;
   }
   return str;
}

CFL_STRP cfl_str_newBufferLen(const char *buffer, CFL_UINT32 len) {
   CFL_STRP str;
   if (buffer == NULL || len == 0) {
      return cfl_str_newConstLen(NULL, 0);
   }
   str = (CFL_STRP) CFL_MEM_ALLOC(sizeof(CFL_STR));
   if (str == NULL) {
      return NULL;
   }
   str->hashValue = 0;
   str->isAllocated = CFL_TRUE;
   str->length = (CFL_UINT32) len;
   str->dataSize = (CFL_UINT32) len + 1;
   str->isVarData = CFL_TRUE;
   str->data = (char *) CFL_MEM_ALLOC(sizeof(char) * str->dataSize);
   if (str->data != NULL) {
      memcpy(str->data, (void *) buffer, len * sizeof(char));
      str->data[len] = '\0';
   } else {
      CFL_MEM_FREE(str);
      str = NULL;
   }
   return str;
}

CFL_STRP cfl_str_newBuffer(const char *buffer) {
   return cfl_str_newBufferLen(buffer, buffer != NULL ? (CFL_UINT32) strlen(buffer) : 0);
}

CFL_STRP cfl_str_newConstLen(const char *buffer, CFL_UINT32 len) {
   CFL_STRP str = (CFL_STRP) CFL_MEM_ALLOC(sizeof(CFL_STR));
   if (str == NULL) {
      return NULL;
   }
   str->isAllocated = CFL_TRUE;
   str->hashValue = 0;
   str->isVarData = CFL_FALSE;
   if (buffer != NULL && len > 0) {
      str->length = (CFL_UINT32) len;
      str->dataSize = (CFL_UINT32) len + 1;
      str->data = (char *) buffer;
   } else {
      str->length = 0;
      str->dataSize = 0;
      str->data = "";
   }
   return str;
}

CFL_STRP cfl_str_newConst(const char *buffer) {
   return cfl_str_newConstLen(buffer, buffer != NULL ? (CFL_UINT32) strlen(buffer) : 0);
}

CFL_STRP cfl_str_newStr(CFL_STRP strSet) {
   CFL_STRP str;
   if (strSet == NULL || strSet->length == 0) {
      return cfl_str_newConstLen(NULL, 0);
   }
   str = (CFL_STRP) CFL_MEM_ALLOC(sizeof(CFL_STR));
   if (str == NULL) {
      return NULL;
   }
   str->isAllocated = CFL_TRUE;
   str->length = strSet->length;
   str->isVarData = strSet->isVarData;
   str->hashValue = strSet->hashValue;
   if (strSet->isVarData) {
      str->dataSize = strSet->length + 1;
      str->data = (char *) CFL_MEM_ALLOC(sizeof(char) * str->dataSize);
      if (str->data != NULL) {
         memcpy(str->data, (void *) strSet->data, strSet->length * sizeof(char));
         str->data[str->length] = '\0';
      } else {
         CFL_MEM_FREE(str);
         str = NULL;
      }
   } else {
      str->dataSize = strSet->dataSize;
      str->data = strSet->data;
   }
   return str;
}

void cfl_str_free(CFL_STRP str) {
   if (str == NULL) {
      return;
   }
   if (str->isVarData && str->data != NULL) {
      CFL_MEM_FREE(str->data);
   }
   if (str->isAllocated) {
      CFL_MEM_FREE(str);
   }
}

CFL_STRP cfl_str_appendChar(CFL_STRP str, char c) {
   CFL_BOOL bSuccess;
   if (str == NULL) {
      str = cfl_str_new(DEFAULT_CAPACITY);
      bSuccess = str != NULL;
   } else {
      bSuccess = ensureCapacityForLen(str, str->length + 1);
   }
   if (bSuccess) {
      str->data[str->length] = c;
      str->length++;
      str->data[str->length] = '\0';
      str->hashValue = 0;
   }
   return str;
}

CFL_STRP cfl_str_append(CFL_STRP str, const char * buffer, ...) {
   va_list va;
   char * strPtr = (char *) buffer;

   if (str == NULL) {
      str = cfl_str_new(DEFAULT_CAPACITY);
   }
   va_start(va, buffer);
   while (strPtr) {
      CFL_UINT32 len = (CFL_UINT32) strlen(strPtr);
      if (len > 0) {
         if (ensureCapacityForLen(str, str->length + len )) {
            memcpy(&str->data[str->length], (void *) strPtr, len * sizeof(char));
            str->length += len;
            str->data[str->length] = '\0';
         }
      }
      strPtr = va_arg(va, char *);
   }
   va_end(va);
   str->hashValue = 0;
   return str;
}

CFL_STRP cfl_str_appendLen(CFL_STRP str, const char *buffer, CFL_UINT32 bufferLen) {
   if (buffer != NULL && bufferLen > 0) {
      if (str == NULL) {
         str = cfl_str_newBufferLen(buffer, bufferLen);
      } else if (ensureCapacityForLen(str, str->length + bufferLen)) {
         memcpy(&str->data[str->length], (void *) buffer, bufferLen * sizeof(char));
         str->length += bufferLen;
         str->data[str->length] = '\0';
         str->hashValue = 0;
      }
   } else if (str == NULL) {
      str = cfl_str_new(DEFAULT_CAPACITY);
   }
   return str;
}

CFL_STRP cfl_str_appendStr(CFL_STRP str, CFL_STRP strAppend) {
   if (strAppend != NULL && strAppend->length > 0) {
      if (str == NULL) {
         str = cfl_str_newStr(strAppend);
      } else if (ensureCapacityForLen(str, str->length + strAppend->length)) {
         memcpy(&str->data[str->length], (void *) strAppend->data, strAppend->length * sizeof(char));
         str->length += strAppend->length;
         str->data[str->length] = '\0';
         str->hashValue = 0;
      }
   } else if (str == NULL) {
      str = cfl_str_new(DEFAULT_CAPACITY);
   }
   return str;
}

CFL_STRP cfl_str_appendFormatArgs(CFL_STRP str, const char * format, va_list varArgs) {
   int iLen;
   va_list varArgsCopy;

   va_copy(varArgsCopy, varArgs);
   iLen = vsnprintf(NULL, 0, format, varArgsCopy);
   va_end(varArgsCopy);
   if (iLen > 0) {
      CFL_BOOL bSuccess;
      if (str == NULL) {
         str = cfl_str_new(iLen);
         bSuccess = str != NULL;
      } else {
         bSuccess = ensureCapacityForLen(str, str->length + iLen);
      }
      if (bSuccess) {
         vsnprintf(&str->data[str->length], iLen + 1, format, varArgs);
         str->length += (CFL_UINT32) iLen;
         str->data[str->length] = '\0';
         str->hashValue = 0;
      }
   }
   return str;
}

CFL_STRP cfl_str_appendFormat(CFL_STRP str, const char * format, ...) {
   va_list varArgs;

   va_start(varArgs, format);
   str = cfl_str_appendFormatArgs(str, format, varArgs);
   va_end(varArgs);
   return str;
}

CFL_STRP cfl_str_setFormatArgs(CFL_STRP str, const char * format, va_list varArgs) {
   int iLen;
   va_list varArgsCopy;

   va_copy(varArgsCopy, varArgs);
   iLen = vsnprintf(NULL, 0, format, varArgsCopy);
   va_end(varArgsCopy);
   if (iLen > 0) {
      CFL_BOOL bSuccess;
      if (str == NULL) {
         str = cfl_str_new(iLen);
         bSuccess = str != NULL;
      } else {
         bSuccess = ensureCapacityForLen(str, iLen);
      }
      if (bSuccess) {
         vsnprintf(str->data, iLen + 1, format, varArgs);
         str->length = (CFL_UINT32) iLen;
         str->data[str->length] = '\0';
         str->hashValue = 0;
      }
   } else {
      str = cfl_str_setConstLen(str, "", 0);
   }
   return str;
}

CFL_STRP cfl_str_setFormat(CFL_STRP str, const char * format, ...) {
   va_list varArgs;

   va_start(varArgs, format);
   str = cfl_str_setFormatArgs(str, format, varArgs);
   va_end(varArgs);
   return str;
}

CFL_STRP cfl_str_setChar(CFL_STRP str, CFL_UINT32 index, char c) {
   if (str == NULL) {
      str = cfl_str_new(index + 1);
      cfl_str_setLength(str, index + 1);
   } else if (index >= str->length) {
      cfl_str_setLength(str, index + 1);
   }
   str->data[index] = c;
   return str;
}

char *cfl_str_getPtr(const CFL_STRP str) {
   return str->data;
}

char *cfl_str_getPtrAt(const CFL_STRP str, CFL_UINT32 index) {
   if (index >= str->length) {
      return NULL;
   }
   return &str->data[index];
}

/* #DEPRECATE. Use cfl_str_length */
CFL_UINT32 cfl_str_getLength(const CFL_STRP str) {
   return str->length;
}

CFL_UINT32 cfl_str_length(const CFL_STRP str) {
   return str->length;
}

void cfl_str_setLength(CFL_STRP str, CFL_UINT32 newLen) {
   if (ensureCapacityForLen(str, newLen)) {
      if (newLen > str->length) {
         memset(&str->data[str->length], ' ', newLen - str->length);
      }
      str->length = newLen;
      str->data[newLen] = '\0';
      str->hashValue = 0;
   }
}

void cfl_str_clear(CFL_STRP str) {
   if (str->isVarData) {
      str->data[0] = '\0';
   } else {
      str->data = "";
      str->dataSize = 0;
   }
   str->hashValue = 0;
   str->length = 0;
}

CFL_STRP cfl_str_setStr(CFL_STRP str, const CFL_STRP src) {
   if (src != NULL && src->length > 0) {
      return cfl_str_setValueLen(str, src->data, src->length);
   } else {
      return cfl_str_setConstLen(str, "", 0);
   }
}

CFL_STRP cfl_str_setValueLen(CFL_STRP str, const char *buffer, CFL_UINT32 bufferLen) {
   if (str == NULL) {
      return cfl_str_newBufferLen(buffer, bufferLen);
   } else if (buffer != NULL && bufferLen > 0) {
      if (ensureCapacityForLen(str, bufferLen)) {
         memcpy(str->data, (void *) buffer, bufferLen * sizeof (char));
         str->length = bufferLen;
         str->hashValue = 0;
         str->data[bufferLen] = '\0';
      }
   } else {
      cfl_str_clear(str);
   }
   return str;
}

CFL_STRP cfl_str_setValue(CFL_STRP str, const char *buffer) {
   return cfl_str_setValueLen(str, buffer, buffer != NULL ? (CFL_UINT32) strlen(buffer) : 0);
}

CFL_STRP cfl_str_setConstLen(CFL_STRP str, const char *buffer, CFL_UINT32 bufferLen) {
   if (str == NULL) {
      return cfl_str_newConstLen(buffer, bufferLen);
   } else {
      if (str->isVarData) {
         CFL_MEM_FREE(str->data);
      }
      if (buffer != NULL && bufferLen > 0) {
         str->data = (char *) buffer;
         str->length = bufferLen;
      } else {
         str->data = "";
         str->length = 0;
      }
      str->dataSize = 0;
      str->hashValue = 0;
      str->isVarData = CFL_FALSE;
      return str;
   }
}

CFL_STRP cfl_str_setConst(CFL_STRP str, const char *buffer) {
   return cfl_str_setConstLen(str, buffer, buffer != NULL ? (CFL_UINT32) strlen(buffer) : 0);
}

CFL_INT16 cfl_str_compare(const CFL_STRP str1, const CFL_STRP str2, CFL_BOOL bExact) {
   char *s1;
   char *s2;
   char c1;
   char c2;

   if (str1 == str2 || str1->data == str2->data) {
      return 0;
   }

   s1 = str1->data;
   s2 = str2->data;
   do {
      c1 = *s1;
      c2 = *s2;

      if (c1 < c2) {
         return c1 == '\0' && !bExact ? 0 : -1;
      } else if (c1 > c2) {
         return c2 == '\0' && !bExact ? 0 : 1;
      }

      s1++;
      s2++;
   } while (c1 != '\0');
   return 0;
}

CFL_INT16 cfl_str_compareIgnoreCase(const CFL_STRP str1, const CFL_STRP str2, CFL_BOOL bExact) {
   char *s1;
   char *s2;
   int c1;
   int c2;

   if (str1 == str2 || str1->data == str2->data) {
      return 0;
   }

   s1 = str1->data;
   s2 = str2->data;
   do {
      c1 = toupper((int) *s1);
      c2 = toupper((int) *s2);

      if (c1 < c2) {
         return c1 == 0 && !bExact ? 0 : -1;
      } else if (c1 > c2) {
         return c2 == 0 && !bExact ? 0 : 1;
      }

      s1++;
      s2++;
   } while (c1);
   return 0;
}

CFL_BOOL cfl_str_startsWith(const CFL_STRP str, const CFL_STRP strStart) {
   if (strStart->length > str->length) {
      return CFL_FALSE;
   }
   return cfl_str_bufferStartsWith(str, strStart->data);
}

CFL_BOOL cfl_str_startsWithIgnoreCase(const CFL_STRP str, const CFL_STRP strStart) {
   if (strStart->length > str->length) {
      return CFL_FALSE;
   }
   return cfl_str_bufferStartsWithIgnoreCase(str, strStart->data);
}

CFL_BOOL cfl_str_bufferStartsWith(const CFL_STRP str, const char *buffer) {
   char *s1;
   char *s2;
   char c1;
   char c2;

   if (str->data == buffer) {
      return CFL_TRUE;
   }

   s1 = str->data;
   s2 = (char *) buffer;
   do {
      c1 = *s1;
      c2 = *s2;

      if (c2 == '\0') {
         return CFL_TRUE;
      } else if (c2 != c1) {
         return CFL_FALSE;
      }
      s1++;
      s2++;
   } while (c1 != '\0');
   return CFL_FALSE;
}

CFL_BOOL cfl_str_bufferStartsWithIgnoreCase(const CFL_STRP str, const char *buffer) {
   char *s1;
   char *s2;
   int c1;
   int c2;

   if (str->data == buffer) {
      return CFL_TRUE;
   }

   s1 = str->data;
   s2 = (char *) buffer;
   do {
      c1 = toupper((int) *s1);
      c2 = toupper((int) *s2);

      if (c2 == 0) {
         return CFL_TRUE;
      } else if (c2 != c1) {
         return CFL_FALSE;
      }
      s1++;
      s2++;
   } while (c1);
   return CFL_FALSE;
}

CFL_BOOL cfl_str_equals(const CFL_STRP str1, const CFL_STRP str2) {
   if (str1->length != str2->length) {
      return CFL_FALSE;
   }
   return cfl_str_compare(str1, str2, CFL_TRUE) == 0;
}

CFL_BOOL cfl_str_equalsIgnoreCase(const CFL_STRP str1, const CFL_STRP str2) {
   if (str1->length != str2->length) {
      return CFL_FALSE;
   }
   return cfl_str_compareIgnoreCase(str1, str2, CFL_TRUE) == 0;
}

CFL_INT16 cfl_str_bufferCompare(const CFL_STRP str, const char *buffer, CFL_BOOL bExact) {
   char *s1;
   char *s2;
   char c1;
   char c2;

   if (str->data == buffer) {
      return 0;
   }

   s1 = str->data;
   s2 = (char *) buffer;
   do {
      c1 = *s1;
      c2 = *s2;

      if (c1 < c2) {
         return c1 == '\0' && !bExact ? 0 : -1;
      } else if (c1 > c2) {
         return c2 == '\0' && !bExact ? 0 : 1;
      }

      s1++;
      s2++;
   } while (c1 != '\0');
   return 0;
}

CFL_INT16 cfl_str_bufferCompareIgnoreCase(const CFL_STRP str, const char *buffer, CFL_BOOL bExact) {
   char *s1;
   char *s2;
   int c1;
   int c2;
   
   if (str->data == buffer) {
      return 0;
   }

   s1 = str->data;
   s2 = (char *) buffer;
   do {
      c1 = toupper((int) *s1);
      c2 = toupper((int) *s2);

      if (c1 < c2) {
         return c1 == '\0' && !bExact ? 0 : -1;
      } else if (c1 > c2) {
         return c2 == '\0' && !bExact ? 0 : 1;
      }

      s1++;
      s2++;
   } while (c1);
   return 0;
}

CFL_BOOL cfl_str_bufferEquals(const CFL_STRP str1, const char *str2) {
   return cfl_str_bufferCompare(str1, str2, CFL_TRUE) == 0;
}

CFL_BOOL cfl_str_bufferEqualsIgnoreCase(const CFL_STRP str1, const char *str2) {
   return cfl_str_bufferCompareIgnoreCase(str1, str2, CFL_TRUE) == 0;
}

CFL_UINT32 cfl_str_hashCode(CFL_STRP str) {
   if (str->hashValue == 0 && str->length > 0) {
      CFL_UINT32 hash = 0;
      CFL_UINT32 i;
      for (i = 0; i < str->length; i++) {
         hash = 31 * hash + str->data[i];
      }
      str->hashValue = hash;
   }
   return str->hashValue;
}

// murmur3 hash algorithm
//CFL_UINT32 cfl_str_hashCode(CFL_STRP str) {
//   if (str->hashValue == 0 && str->length > 0) {
//      CFL_UINT8* key = str->data;
//      CFL_UINT32 hash = (CFL_UINT32) key[0];
//      size_t len = (size_t) str->length;
//      if (len > 3) {
//         const CFL_UINT32* key_x4 = (const CFL_UINT32*) key;
//         size_t i = len >> 2;
//         do {
//            CFL_UINT32 k = *key_x4++;
//            k *= 0xcc9e2d51;
//            k = (k << 15) | (k >> 17);
//            k *= 0x1b873593;
//            hash ^= k;
//            hash = (hash << 13) | (hash >> 19);
//            hash = (hash * 5) + 0xe6546b64;
//         } while (--i);
//         key = (const CFL_UINT8*) key_x4;
//      }
//      if (len & 3) {
//         size_t i = len & 3;
//         CFL_UINT32 k = 0;
//         key = &key[i - 1];
//         do {
//            k <<= 8;
//            k |= *key--;
//         } while (--i);
//         k *= 0xcc9e2d51;
//         k = (k << 15) | (k >> 17);
//         k *= 0x1b873593;
//         hash ^= k;
//      }
//      hash ^= len;
//      hash ^= hash >> 16;
//      hash *= 0x85ebca6b;
//      hash ^= hash >> 13;
//      hash *= 0xc2b2ae35;
//      hash ^= hash >> 16;
//      str->hashValue = hash;
//   }
//   return str->hashValue;
//}

CFL_STRP cfl_str_toUpper(CFL_STRP str) {
   CFL_UINT32 i;
   for (i = 0; i < str->length; i++) {
      if (CFL_ISLOWER(str->data[i])) {
         str->data[i] = CFL_TOUPPER(str->data[i]);
         str->hashValue = 0;
      }
   }
   return str;
}

CFL_STRP cfl_str_toLower(CFL_STRP str) {
   CFL_UINT32 i;
   for (i = 0; i < str->length; i++) {
      if (CFL_ISUPPER(str->data[i])) {
         str->data[i] = CFL_TOLOWER(str->data[i]);
         str ->hashValue = 0;
      }
   }
   return str;
}

CFL_STRP cfl_str_trim(CFL_STRP str) {
   if (str->length > 0) {
      CFL_UINT32 start = 0;
      CFL_UINT32 end;
      while (str->data[start] && CFL_ISSPACE(str->data[start])) {
         ++start;
      }
      end = str->length - 1;
      while (CFL_ISSPACE(str->data[end]) && end > start) {
         --end;
      }
      ++end;
      if (start > 0 || end < str->length) {
         if (start > end) {
            memmove(str->data, &str->data[start], end - start);
         }
         end -= start;
         if (end < str->length) {
            str->data[end] = '\0';
            str->length = end;
         }
         str->hashValue = 0;
      }
   }
   return str;
}

CFL_BOOL cfl_str_isEmpty(const CFL_STRP str) {
   if (str && str->length > 0) {
      return CFL_FALSE;
   }
   return CFL_TRUE;
}

CFL_BOOL cfl_str_isBlank(const CFL_STRP str) {
   if (str && str->length > 0) {
      CFL_UINT32 i;
      for (i = 0; i < str->length; i++) {
         if (! isspace(str->data[i])) {
            return CFL_FALSE;
         }
      }
   }
   return CFL_TRUE;
}

CFL_STRP cfl_str_substr(const CFL_STRP str, CFL_UINT32 start, CFL_UINT32 end) {
   CFL_STRP subs;
   if (start < str->length) {
      if (end > str->length) {
         subs = cfl_str_newBufferLen(&str->data[start], str->length - start);
      } else {
         subs = cfl_str_newBufferLen(&str->data[start], end - start);
      }
   } else {
      subs = cfl_str_newConstLen(NULL, 0);
   }
   return subs;
}


CFL_INT32 cfl_str_indexOf(const CFL_STRP str, char search, CFL_UINT32 start) {
   CFL_UINT32 i;
   for (i = start; i < str->length; i++) {
      if (search == str->data[i]) {
         return i <= 0x0FFFFFFF ? (CFL_INT32) i : -1;
      }
   }
   return -1;
}

CFL_INT32 cfl_str_indexOfBuffer(const CFL_STRP str, const char *search, CFL_UINT32 searchLen, CFL_UINT32 start) {
   CFL_UINT32 index;
   CFL_UINT32 maxLen;
   if (searchLen == 0 || str->length - start < searchLen) {
      return -1;
   }
   index = start;
   maxLen = str->length - searchLen;
   do {
      if (str->data[index] == search[0]) {
         CFL_UINT32 indexSearch = searchLen;
         do {
            if (--indexSearch == 0) {
               return index <= 0x0FFFFFFF ? (CFL_INT32) index : -1;
            }
         } while (str->data[index + indexSearch] == search[indexSearch]);
      }
   } while (index++ < maxLen);
   return -1;
}

CFL_INT32 cfl_str_indexOfStr(const CFL_STRP str, const CFL_STRP search, CFL_UINT32 start) {
   return cfl_str_indexOfBuffer(str, search->data, search->length, start);
}

char cfl_str_charAt(const CFL_STRP str, CFL_UINT32 index) {
   return index < str->length ? str->data[index] : '\0';
}

char cfl_str_charRAt(const CFL_STRP str, CFL_UINT32 index) {
   return index < str->length ? str->data[str->length - index - 1] : '\0';
}

CFL_UINT32 cfl_str_replaceChar(CFL_STRP str, char oldChar, char newChar) {
   CFL_UINT32 i;
   CFL_UINT32 count = 0;
   for (i = 0; i < str->length; i++) {
      if (str->data[i] == oldChar) {
         str->data[i] = newChar;
         ++count;
      }
   }
   return count;
}

CFL_STRP cfl_str_copyBufferLen(CFL_STRP dest, const char *source, CFL_UINT32 sourceLen, CFL_UINT32 start, CFL_UINT32 end) {
   CFL_UINT32 index;
   
   if (start >= sourceLen) {
      if (dest == NULL) {
         dest = cfl_str_newConstLen("", 0);
      } else { 
         cfl_str_setLength(dest, 0);
      }
      return dest;
   } else if (end < start || end > sourceLen) {
      end = sourceLen;
   }

   if (dest == NULL) {
      dest = cfl_str_new(end - start + 1);
   } else {
     ensureCapacityForLen(dest, end - start);
   }
   index = 0;
   while (start < end) {
      dest->data[index++] = source[start++];
   }
   dest->data[index] = '\0';
   dest->length = index;
   dest->hashValue = 0;
   return dest;
}

CFL_STRP cfl_str_copyBuffer(CFL_STRP dest, const char *source, CFL_UINT32 start, CFL_UINT32 end) {
   return cfl_str_copyBufferLen(dest, source, (CFL_UINT32) strlen(source), start, end);
}

CFL_STRP cfl_str_copy(CFL_STRP dest, const CFL_STRP source, CFL_UINT32 start, CFL_UINT32 end) {
   return cfl_str_copyBufferLen(dest, source->data, source->length, start, end);
}

CFL_STRP cfl_str_move(CFL_STRP dest, CFL_STRP source) {
   if (dest->isVarData && dest->data != NULL) {
      CFL_MEM_FREE(dest->data);
   }
   dest->data = source->data;
   dest->length = source->length;
   dest->dataSize = source->dataSize;
   dest->hashValue = source->hashValue;
   dest->isVarData = source->isVarData;
   source->dataSize = 0;
   source->length = 0;
   source->hashValue = 0;
   source->isVarData = CFL_FALSE;
   source->data = "";
   return dest;
}
