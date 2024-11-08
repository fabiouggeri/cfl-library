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

#ifndef CFL_STR_H_

#define CFL_STR_H_

#include <stdarg.h>
#include "cfl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define CFL_STR_FREE(s) if (s != NULL) { cfl_str_free(s); s = NULL; }
#define CFL_STR_APPEND_CONST(s, c) cfl_str_appendLen(s, c, (int) sizeof(c) - 1)
#define CFL_STR_SET_CONST(s, c)    cfl_str_setConstLen(s, c, (int) sizeof(c) - 1)

#define CFL_ISUPPER( c ) ( ( c ) >= 'A' && ( c ) <= 'Z' )
#define CFL_ISLOWER( c ) ( ( c ) >= 'a' && ( c ) <= 'z' )
#define CFL_ISDIGIT( c ) ( ( c ) >= '0' && ( c ) <= '9' )
#define CFL_TOUPPER( c ) ( ( c ) - ( 'a' - 'A' ) )
#define CFL_TOLOWER( c ) ( ( c ) + ( 'a' - 'A' ) )
#define CFL_ISSPACE( c ) isspace( c )

#define CFL_STR_EMPTY    { "", 0            , 1        , 0, CFL_FALSE, CFL_FALSE }
#define CFL_STR_CONST(s) { s , sizeof(s) - 1, sizeof(s), 0, CFL_FALSE, CFL_FALSE }

typedef struct _CFL_STR {
   char *data;
   CFL_UINT32 length;
   CFL_UINT32 dataSize;
   CFL_UINT32 hashValue;
   CFL_BOOL   isVarData;
   CFL_BOOL   isAllocated;
} CFL_STR, *CFL_STRP;

extern void cfl_str_init(CFL_STRP str);
extern void cfl_str_initCapacity(CFL_STRP str, CFL_UINT32 iniCapacity);
extern void cfl_str_initConstLen(CFL_STRP str, const char *buffer, CFL_UINT32 len);
extern void cfl_str_initConst(CFL_STRP str, const char *buffer);
extern void cfl_str_initValue(CFL_STRP str, const char *buffer);
extern CFL_STRP cfl_str_new(CFL_UINT32  iniCapacity);
extern CFL_STRP cfl_str_newBuffer(const char *buffer);
extern CFL_STRP cfl_str_newBufferLen(const char *buffer, CFL_UINT32 len);
extern CFL_STRP cfl_str_newConst(const char *buffer);
extern CFL_STRP cfl_str_newConstLen(const char *buffer, CFL_UINT32 len);
extern CFL_STRP cfl_str_newStr(CFL_STRP str);
extern void cfl_str_free(CFL_STRP str);
extern CFL_STRP cfl_str_append(CFL_STRP str, const char * buffer, ...);
extern CFL_STRP cfl_str_appendChar(CFL_STRP str, char c);
extern CFL_STRP cfl_str_appendLen(CFL_STRP str, const char *buffer, CFL_UINT32 len);
extern CFL_STRP cfl_str_appendStr(CFL_STRP str, CFL_STRP strAppend);
extern CFL_STRP cfl_str_appendFormatArgs(CFL_STRP str, const char * format, va_list varArgs);
extern CFL_STRP cfl_str_appendFormat(CFL_STRP str, const char * format, ...);

extern char *cfl_str_getPtr(const CFL_STRP str);
extern char *cfl_str_getPtrAt(const CFL_STRP str, CFL_UINT32 index);
extern CFL_UINT32 cfl_str_getLength(const CFL_STRP str);
extern CFL_UINT32 cfl_str_length(const CFL_STRP str);
extern void cfl_str_setLength(CFL_STRP str, CFL_UINT32 newLen);
extern void cfl_str_clear(CFL_STRP str);
extern CFL_STRP cfl_str_setStr(CFL_STRP str, const CFL_STRP src);
extern CFL_STRP cfl_str_setValue(CFL_STRP str, const char *buffer);
extern CFL_STRP cfl_str_setValueLen(CFL_STRP str, const char *buffer, CFL_UINT32 len);
extern CFL_STRP cfl_str_setConst(CFL_STRP str, const char *buffer);
extern CFL_STRP cfl_str_setConstLen(CFL_STRP str, const char *buffer, CFL_UINT32 len);
extern CFL_STRP cfl_str_setFormatArgs(CFL_STRP str, const char * format, va_list varArgs);
extern CFL_STRP cfl_str_setFormat(CFL_STRP str, const char * format, ...);
extern CFL_STRP cfl_str_setChar(CFL_STRP str, CFL_UINT32 index, char c);

extern CFL_BOOL cfl_str_startsWith(const CFL_STRP str, const CFL_STRP strStart);
extern CFL_BOOL cfl_str_startsWithIgnoreCase(const CFL_STRP str, const CFL_STRP strStart);
extern CFL_BOOL cfl_str_bufferStartsWith(const CFL_STRP str, const char *buffer);
extern CFL_BOOL cfl_str_bufferStartsWithIgnoreCase(const CFL_STRP str, const char *buffer);

extern CFL_BOOL cfl_str_equals(const CFL_STRP str1, const CFL_STRP str2);
extern CFL_BOOL cfl_str_equalsIgnoreCase(const CFL_STRP str1, const CFL_STRP str2);
extern CFL_INT16 cfl_str_compare(const CFL_STRP str1, const CFL_STRP str2, CFL_BOOL bExact);
extern CFL_INT16 cfl_str_compareIgnoreCase(const CFL_STRP str1, const CFL_STRP str2, CFL_BOOL bExact);

extern CFL_BOOL cfl_str_bufferEquals(const CFL_STRP str1, const char *str2);
extern CFL_BOOL cfl_str_bufferEqualsIgnoreCase(const CFL_STRP str1, const char *str2);
extern CFL_INT16 cfl_str_bufferCompare(const CFL_STRP str1, const char *str2, CFL_BOOL bExact);
extern CFL_INT16 cfl_str_bufferCompareIgnoreCase(const CFL_STRP str1, const char *str2, CFL_BOOL bExact);

extern CFL_UINT32 cfl_str_hashCode(CFL_STRP str);
extern CFL_STRP cfl_str_toUpper(CFL_STRP str);
extern CFL_STRP cfl_str_toLower(CFL_STRP str);
extern CFL_STRP cfl_str_trim(CFL_STRP str);

extern CFL_BOOL cfl_str_isEmpty(const CFL_STRP str);
extern CFL_BOOL cfl_str_isBlank(const CFL_STRP str);
extern CFL_STRP cfl_str_substr(const CFL_STRP str, CFL_UINT32 start, CFL_UINT32 end);

extern CFL_INT32 cfl_str_indexOf(const CFL_STRP str, char search, CFL_UINT32 start);
extern CFL_INT32 cfl_str_indexOfStr(const CFL_STRP str, const CFL_STRP search, CFL_UINT32 start);
extern CFL_INT32 cfl_str_indexOfBuffer(const CFL_STRP str, const char *search, CFL_UINT32 searchLen, CFL_UINT32 start);

extern char cfl_str_charAt(const CFL_STRP str, CFL_UINT32 index);
extern char cfl_str_charRAt(const CFL_STRP str, CFL_UINT32 index);

extern CFL_UINT32 cfl_str_replaceChar(CFL_STRP str, char oldChar, char newChar);

extern CFL_STRP cfl_str_copyBufferLen(CFL_STRP dest, const char *source, CFL_UINT32 sourceLen, CFL_UINT32 start, CFL_UINT32 end);
extern CFL_STRP cfl_str_copyBuffer(CFL_STRP dest, const char *source, CFL_UINT32 start, CFL_UINT32 end);
extern CFL_STRP cfl_str_copy(CFL_STRP dest, const CFL_STRP source, CFL_UINT32 start, CFL_UINT32 end);

extern CFL_STRP cfl_str_move(CFL_STRP dest, CFL_STRP source);

#ifdef __cplusplus
}
#endif

#endif
