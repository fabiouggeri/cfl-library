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

#define DEFAULT_CAPACITY 16

#define FORMAT_BUFFER_SIZE 4096

static CFL_BOOL ensureCapacityForLen(CFL_STRP str, CFL_UINT32 newLen) {
   if (! str->isVarData) {
      char *curData = str->data;
      str->ulCapacity = (newLen >> 1) + 1 + newLen;
      str->data = (char *) malloc(sizeof(char) * str->ulCapacity);
      str->isVarData = CFL_TRUE;
      if (str->data != NULL) {
         memcpy(str->data, curData, str->ulLength);
         str->data[str->ulLength] = '\0';
      }
   } else if (newLen >= str->ulCapacity) {
      str->ulCapacity = (str->ulCapacity >> 1) + 1 + newLen;
      str->data = (char *) realloc(str->data, str->ulCapacity);
   }
   return str->data != NULL;
}

CFL_STRP cfl_str_new(CFL_UINT32 iniCapacity) {
   CFL_STRP str;
   str = (CFL_STRP) malloc(sizeof(CFL_STR));
   if (str != NULL) {
      str->ulCapacity = iniCapacity;
      str->ulLength = 0;
      str->ulHash = 0;
      str->isVarData = CFL_TRUE;
      str->data = (char *) malloc(iniCapacity * sizeof(char));
      if (str->data != NULL) {
         str->data[0] = '\0';
      } else {
         free(str);
         str = NULL;
      }
   }
   return str;
}

CFL_STRP cfl_str_newBufferLen(const char *buffer, CFL_UINT32 len) {
   CFL_STRP str;
   if (buffer != NULL && len >= 0) {
      str = (CFL_STRP) malloc(sizeof(CFL_STR));
      if (str != NULL) {
         str->ulCapacity = (CFL_UINT32) len + 1;
         str->ulLength = (CFL_UINT32) len;
         str->ulHash = 0;
         str->isVarData = CFL_TRUE;
         str->data = (char *) malloc(sizeof(char) * str->ulCapacity);
         if (str->data != NULL) {
            memcpy(str->data, (void *) buffer, len * sizeof(char));
            str->data[len] = '\0';
         } else {
            free(str);
            str = NULL;
         }
      }
   } else {
      str = cfl_str_new(DEFAULT_CAPACITY);
   }
   return str;
}

CFL_STRP cfl_str_newBuffer(const char *buffer) {
   return cfl_str_newBufferLen(buffer, buffer != NULL ? (CFL_UINT32) strlen(buffer) : 0);
}

CFL_STRP cfl_str_newConstLen(const char *buffer, CFL_UINT32 len) {
   CFL_STRP str = (CFL_STRP) malloc(sizeof(CFL_STR));
   if (str != NULL) {
      str->ulCapacity = 0;
      str->ulHash = 0;
      if (buffer != NULL && len > 0) {
         str->data = (char *) buffer;
         str->ulLength = (CFL_UINT32) len;
      } else {
         str->data = "";
         str->ulLength = 0;
      }
      str->isVarData = CFL_FALSE;
   }
   return str;
}

CFL_STRP cfl_str_newConst(const char *buffer) {
   return cfl_str_newConstLen(buffer, buffer != NULL ? (CFL_UINT32) strlen(buffer) : 0);
}

CFL_STRP cfl_str_newStr(CFL_STRP strSet) {
   CFL_STRP str;
   if (strSet != NULL && strSet->ulLength > 0) {
      str = (CFL_STRP) malloc(sizeof(CFL_STR));
      if (str != NULL) {
         str->ulCapacity = strSet->ulLength + 1;
         str->ulLength = strSet->ulLength;
         str->ulHash = 0;
         str->data = (char *) malloc(sizeof(char) * str->ulCapacity);
         str->isVarData = CFL_TRUE;
         if (str->data != NULL) {
            memcpy(str->data, (void *) strSet->data, strSet->ulLength * sizeof(char));
            str->data[str->ulLength] = '\0';
         } else {
            free(str);
            str = NULL;
         }
      }
   } else {
      str = cfl_str_new(DEFAULT_CAPACITY);
   }
   return str;
}

void cfl_str_free(CFL_STRP str) {
   if (str != NULL) {
      if (str->isVarData && str->data != NULL) {
         free(str->data);
      }
      free(str);
   }
}

CFL_STRP cfl_str_appendChar(CFL_STRP str, char c) {
   CFL_BOOL bSuccess;
   if (str == NULL) {
      str = cfl_str_new(DEFAULT_CAPACITY);
      bSuccess = str != NULL;
   } else {
      bSuccess = ensureCapacityForLen(str, str->ulLength + 1);
   }
   if (bSuccess) {
      str->data[(str->ulLength)++] = c;
      str->data[str->ulLength] = '\0';
      str->ulHash = 0;
   }
   return str;
}

CFL_STRP cfl_str_append(CFL_STRP str, const char * buffer, ...) {
   CFL_UINT32 len;
   va_list va;
   char * strPtr = (char *) buffer;

   if (str == NULL) {
      str = cfl_str_new(DEFAULT_CAPACITY);
   }
   va_start(va, buffer);
   while (strPtr) {
      len = (CFL_UINT32) strlen(strPtr);
      if (len > 0) {
         if (ensureCapacityForLen(str, str->ulLength + len )) {
            memcpy(&str->data[str->ulLength], (void *) strPtr, len * sizeof(char));
            str->ulLength += len;
            str->data[str->ulLength] = '\0';
         }
      }
      strPtr = va_arg(va, char *);
   }
   va_end(va);
   str->ulHash = 0;
   return str;
}

CFL_STRP cfl_str_appendLen(CFL_STRP str, const char *buffer, int bufferLen) {
   if (buffer != NULL && bufferLen > 0) {
      if (str == NULL) {
         str = cfl_str_newBufferLen(buffer, bufferLen);
      } else if (ensureCapacityForLen(str, str->ulLength + bufferLen)) {
         memcpy(&str->data[str->ulLength], (void *) buffer, bufferLen * sizeof(char));
         str->ulLength += bufferLen;
         str->data[str->ulLength] = '\0';
         str->ulHash = 0;
      }
   } else if (str == NULL) {
      str = cfl_str_new(DEFAULT_CAPACITY);
   }
   return str;
}

CFL_STRP cfl_str_appendStr(CFL_STRP str, CFL_STRP strAppend) {
   if (strAppend != NULL && strAppend->ulLength > 0) {
      if (str == NULL) {
         str = cfl_str_newStr(strAppend);
      } else if (ensureCapacityForLen(str, str->ulLength + strAppend->ulLength)) {
         memcpy(&str->data[str->ulLength], (void *) strAppend->data, strAppend->ulLength * sizeof(char));
         str->ulLength += strAppend->ulLength;
         str->data[str->ulLength] = '\0';
         str->ulHash = 0;
      }
   } else if (str == NULL) {
      str = cfl_str_new(DEFAULT_CAPACITY);
   }
   return str;
}

CFL_STRP cfl_str_appendFormatArgs(CFL_STRP str, const char * format, va_list varArgs) {
   int iLen;
   char buffer[FORMAT_BUFFER_SIZE];

   iLen = vsnprintf(buffer, FORMAT_BUFFER_SIZE, format, varArgs);
   if (iLen > 0) {
      if (iLen < FORMAT_BUFFER_SIZE) {
         str = cfl_str_appendLen(str, buffer, iLen);
      } else {
         CFL_BOOL bSuccess;
         if (str == NULL) {
            str = cfl_str_new((iLen >> 1) + 1 + iLen);
            bSuccess = str != NULL;
         } else {
            bSuccess = ensureCapacityForLen(str, str->ulLength + iLen);
         }
         if (bSuccess) {
            iLen = vsnprintf(&str->data[str->ulLength], iLen, format, varArgs);
            str->ulLength += iLen;
            str->data[str->ulLength] = '\0';
            str->ulHash = 0;
         }
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
   char buffer[FORMAT_BUFFER_SIZE];

   iLen = vsnprintf(buffer, FORMAT_BUFFER_SIZE, format, varArgs);
   if (iLen > 0) {
      if (iLen < FORMAT_BUFFER_SIZE) {
         str = cfl_str_setValueLen(str, buffer, iLen);
      } else {
         CFL_BOOL bSuccess;
         if (str == NULL) {
            str = cfl_str_new((iLen >> 1) + 1 + iLen);
            bSuccess = str != NULL;
         } else {
            bSuccess = ensureCapacityForLen(str, iLen);
         }
         if (bSuccess) {
            iLen = vsnprintf(str->data, iLen, format, varArgs);
            str->ulLength = iLen;
            str->data[iLen] = '\0';
            str->ulHash = 0;
         }
      }
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

char *cfl_str_getPtr(CFL_STRP str) {
   return str->data;
}

CFL_UINT32 cfl_str_getLength(CFL_STRP str) {
   return str->ulLength;
}

void cfl_str_clear(CFL_STRP str) {
   if (str->isVarData) {
      str->ulLength = 0;
      str->ulHash = 0;
      str->data[0] = '\0';
   } else {
      str->ulCapacity = 1;
      str->ulLength = 0;
      str->data = "";
      str->ulHash = 0;
   }
}

CFL_STRP cfl_str_setStr(CFL_STRP str, CFL_STRP strSet) {
   if (strSet != NULL && strSet->ulLength > 0) {
      return cfl_str_setValueLen(str, strSet->data, strSet->ulLength);
   } else {
      return cfl_str_setConstLen(str, "", 0);
   }
}

CFL_STRP cfl_str_setValueLen(CFL_STRP str, const char *buffer, CFL_UINT32 bufferLen) {
   if (str == NULL) {
      return cfl_str_newBufferLen(buffer, bufferLen);
   } else if (buffer != NULL && bufferLen > 0) {
      if (ensureCapacityForLen(str, bufferLen)) {
         if (bufferLen > 0) {
            memcpy(str->data, (void *) buffer, bufferLen * sizeof (char));
         }
         str->ulLength = bufferLen;
         str->ulHash = 0;
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
         free(str->data);
      }
      if (buffer != NULL && bufferLen > 0) {
         str->data = (char *) buffer;
         str->ulLength = bufferLen;
      } else {
         str->data = "";
         str->ulLength = 0;
      }
      str->ulCapacity = 0;
      str->ulHash = 0;
      str->isVarData = CFL_FALSE;
      return str;
   }
}

CFL_STRP cfl_str_setConst(CFL_STRP str, const char *buffer) {
   return cfl_str_setConstLen(str, buffer, buffer != NULL ? (CFL_UINT32) strlen(buffer) : 0);
}

CFL_INT16 cfl_str_compare(CFL_STRP str1, CFL_STRP str2, CFL_BOOL bExact) {
   char *s1;
   char *s2;
   char c1;
   char c2;

   if (str1 == str2) {
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

CFL_INT16 cfl_str_compareIgnoreCase(CFL_STRP str1, CFL_STRP str2, CFL_BOOL bExact) {
   char *s1;
   char *s2;
   int c1;
   int c2;

   if (str1 == str2) {
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

CFL_BOOL cfl_str_bufferStartsWith(CFL_STRP str, const char *buffer) {
   char *s1 = str->data;
   char *s2 = (char *) buffer;
   char c1;
   char c2;

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

CFL_BOOL cfl_str_bufferStartsWithIgnoreCase(CFL_STRP str, const char *buffer) {
   char *s1 = str->data;
   char *s2 = (char *) buffer;
   int c1;
   int c2;

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

CFL_BOOL cfl_str_equals(CFL_STRP str1, CFL_STRP str2) {
   if (str1->ulLength != str2->ulLength) {
      return CFL_FALSE;
   }
   return cfl_str_compare(str1, str2, CFL_TRUE) == 0;
}

CFL_BOOL cfl_str_equalsIgnoreCase(CFL_STRP str1, CFL_STRP str2) {
   if (str1->ulLength != str2->ulLength) {
      return CFL_FALSE;
   }
   return cfl_str_compareIgnoreCase(str1, str2, CFL_TRUE) == 0;
}

CFL_INT16 cfl_str_bufferCompare(CFL_STRP str1, const char *s2, CFL_BOOL bExact) {
   char *s1 = str1->data;
   char c1;
   char c2;

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

CFL_INT16 cfl_str_bufferCompareIgnoreCase(CFL_STRP str1, const char *s2, CFL_BOOL bExact) {
   char *s1 = str1->data;
   int c1;
   int c2;

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

CFL_BOOL cfl_str_bufferEquals(CFL_STRP str1, const char *str2) {
   return cfl_str_bufferCompare(str1, str2, CFL_TRUE) == 0;
}

CFL_BOOL cfl_str_bufferEqualsIgnoreCase(CFL_STRP str1, const char *str2) {
   return cfl_str_bufferCompareIgnoreCase(str1, str2, CFL_TRUE) == 0;
}

CFL_UINT32 cfl_str_hashCode(CFL_STRP str) {
   if (str->ulHash == 0 && str->ulLength > 0) {
      CFL_UINT32 hash = 0;
      CFL_UINT32 i;
      for (i = 0; i < str->ulLength; i++) {
         hash = 31 * hash + str->data[i];
      }
      str->ulHash = hash;
   }
   return str->ulHash;
}

// murmur3 hash algorithm
//CFL_UINT32 cfl_str_hashCode(CFL_STRP str) {
//   if (str->ulHash == 0 && str->ulLength > 0) {
//      CFL_UINT8* key = str->data;
//      CFL_UINT32 hash = (CFL_UINT32) key[0];
//      size_t len = (size_t) str->ulLength;
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
//      str->ulHash = hash;
//   }
//   return str->ulHash;
//}

CFL_STRP cfl_str_toUpper(CFL_STRP str) {
   CFL_UINT32 i;
   for (i = 0; i < str->ulLength; i++) {
      if (CFL_ISLOWER(str->data[i])) {
         str->data[i] = CFL_TOUPPER(str->data[i]);
      }
   }
   return str;
}

CFL_STRP cfl_str_toLower(CFL_STRP str) {
   CFL_UINT32 i;
   for (i = 0; i < str->ulLength; i++) {
      if (CFL_ISUPPER(str->data[i])) {
         str->data[i] = CFL_TOLOWER(str->data[i]);
      }
   }
   return str;
}

CFL_STRP cfl_str_trim(CFL_STRP str) {
   if (str->ulLength > 0) {
      CFL_UINT32 start = 0;
      CFL_UINT32 end;
      while (str->data[start] && CFL_ISSPACE(str->data[start])) {
         ++start;
      }
      end = str->ulLength - 1;
      while (CFL_ISSPACE(str->data[end]) && end > start) {
         --end;
      }
      ++end;
      if (start > 0 || end < str->ulLength) {
         if (start > end) {
            memmove(str->data, &str->data[start], end - start);
         }
         end -= start;
         if (end < str->ulLength) {
            str->data[end] = '\0';
            str->ulLength = end;
         }
      }
   }
   return str;
}

CFL_BOOL cfl_str_isEmpty(CFL_STRP str) {
   if (str && str->ulLength > 0) {
      return CFL_FALSE;
   }
   return CFL_TRUE;
}

CFL_BOOL cfl_str_isBlank(CFL_STRP str) {
   if (str && str->ulLength > 0) {
      CFL_UINT32 i;
      for (i = 0; i < str->ulLength; i++) {
         if (! isspace(str->data[i])) {
            return CFL_FALSE;
         }
      }
   }
   return CFL_TRUE;
}

CFL_STRP cfl_str_substr(CFL_STRP str, CFL_UINT32 start, CFL_UINT32 end) {
   CFL_STRP subs;
   if (start < str->ulLength) {
      if (end > str->ulLength) {
         subs = cfl_str_newBufferLen(&str->data[start], str->ulLength - start);
      } else {
         subs = cfl_str_newBufferLen(&str->data[start], end - start);
      }
   } else {
      subs = cfl_str_newConstLen(NULL, 0);
   }
   return subs;
}
