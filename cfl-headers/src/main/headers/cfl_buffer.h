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

#ifndef _CFL_BUFFER_H_

#define _CFL_BUFFER_H_

#include "cfl_types.h"
#include "cfl_date.h"

#define CFL_BIG_ENDIAN    0x00
#define CFL_LITTLE_ENDIAN 0x01

struct _CFL_BUFFER {
	CFL_UINT8  *data;
	CFL_UINT32 length;
	CFL_UINT32 position;
	CFL_UINT32 capacity;
	CFL_BOOL   allocated;
   CFL_UINT8  endian;
};

extern void cfl_buffer_init(CFL_BUFFERP buffer);
extern CFL_BUFFERP cfl_buffer_new(void);
extern CFL_BUFFERP cfl_buffer_newCapacity(CFL_UINT32 initialCapacity);
extern CFL_BUFFERP cfl_buffer_clone(CFL_BUFFERP other);
extern void cfl_buffer_free(CFL_BUFFERP buffer);
extern CFL_UINT8 *cfl_buffer_getDataPtr(CFL_BUFFERP buffer);
extern CFL_UINT8 *cfl_buffer_positionPtr(CFL_BUFFERP buffer);
extern void cfl_buffer_setBigEndian(CFL_BUFFERP buffer, CFL_BOOL bigEndian);
extern CFL_BOOL cfl_buffer_isBigEndian(CFL_BUFFERP buffer);
extern CFL_UINT32 cfl_buffer_length(CFL_BUFFERP buffer);
extern CFL_BOOL cfl_buffer_setLength(CFL_BUFFERP buffer, CFL_UINT32 newLen);
extern void cfl_buffer_flip(CFL_BUFFERP buffer);
extern void cfl_buffer_reset(CFL_BUFFERP buffer);
extern CFL_UINT32 cfl_buffer_position(CFL_BUFFERP buffer);
extern void cfl_buffer_setPosition(CFL_BUFFERP buffer, CFL_UINT32 newPos);
extern void cfl_buffer_skip(CFL_BUFFERP buffer, CFL_UINT32 skip);
extern void cfl_buffer_rewind(CFL_BUFFERP buffer);
extern void cfl_buffer_compact(CFL_BUFFERP buffer);
extern CFL_UINT32 cfl_buffer_capacity(CFL_BUFFERP buffer);
extern CFL_BOOL cfl_buffer_setCapacity(CFL_BUFFERP buffer, CFL_UINT32 newCapacity);
extern CFL_BOOL cfl_buffer_putBoolean(CFL_BUFFERP buffer, CFL_BOOL value);
extern CFL_BOOL cfl_buffer_getBoolean(CFL_BUFFERP buffer);
extern CFL_BOOL cfl_buffer_peekBoolean(CFL_BUFFERP buffer);
extern CFL_BOOL cfl_buffer_putInt8(CFL_BUFFERP buffer, CFL_INT8 value);
extern CFL_INT8 cfl_buffer_getInt8(CFL_BUFFERP buffer);
extern CFL_INT8 cfl_buffer_peekInt8(CFL_BUFFERP buffer);
extern CFL_BOOL cfl_buffer_putInt16(CFL_BUFFERP buffer, CFL_INT16 value);
extern CFL_INT16 cfl_buffer_getInt16(CFL_BUFFERP buffer);
extern CFL_INT16 cfl_buffer_peekInt16(CFL_BUFFERP buffer);
extern CFL_BOOL cfl_buffer_putInt32(CFL_BUFFERP buffer, CFL_INT32 value);
extern CFL_INT32 cfl_buffer_getInt32(CFL_BUFFERP buffer);
extern CFL_INT32 cfl_buffer_peekInt32(CFL_BUFFERP buffer);
extern CFL_BOOL cfl_buffer_putInt64(CFL_BUFFERP buffer, CFL_INT64 value);
extern CFL_INT64 cfl_buffer_getInt64(CFL_BUFFERP buffer);
extern CFL_INT64 cfl_buffer_peekInt64(CFL_BUFFERP buffer);
extern CFL_BOOL cfl_buffer_putUInt8(CFL_BUFFERP buffer, CFL_UINT8 value);
extern CFL_UINT8 cfl_buffer_getUInt8(CFL_BUFFERP buffer);
extern CFL_UINT8 cfl_buffer_peekUInt8(CFL_BUFFERP buffer);
extern CFL_BOOL cfl_buffer_putUInt16(CFL_BUFFERP buffer, CFL_UINT16 value);
extern CFL_UINT16 cfl_buffer_getUInt16(CFL_BUFFERP buffer);
extern CFL_UINT16 cfl_buffer_peekUInt16(CFL_BUFFERP buffer);
extern CFL_BOOL cfl_buffer_putUInt32(CFL_BUFFERP buffer, CFL_UINT32 value);
extern CFL_UINT32 cfl_buffer_getUInt32(CFL_BUFFERP buffer);
extern CFL_UINT32 cfl_buffer_peekUInt32(CFL_BUFFERP buffer);
extern CFL_BOOL cfl_buffer_putUInt64(CFL_BUFFERP buffer, CFL_UINT64 value);
extern CFL_UINT64 cfl_buffer_getUInt64(CFL_BUFFERP buffer);
extern CFL_UINT64 cfl_buffer_peekUInt64(CFL_BUFFERP buffer);
extern CFL_BOOL cfl_buffer_putFloat(CFL_BUFFERP buffer, float value);
extern float cfl_buffer_getFloat(CFL_BUFFERP buffer);
extern float cfl_buffer_peekFloat(CFL_BUFFERP buffer);
extern CFL_BOOL cfl_buffer_putDouble(CFL_BUFFERP buffer, double value);
extern double cfl_buffer_getDouble(CFL_BUFFERP buffer);
extern double cfl_buffer_peekDouble(CFL_BUFFERP buffer);
extern CFL_STRP cfl_buffer_getString(CFL_BUFFERP buffer);
extern CFL_UINT32 cfl_buffer_getStringLength(CFL_BUFFERP buffer);
extern void cfl_buffer_copyString(CFL_BUFFERP buffer, CFL_STRP destStr);
extern void cfl_buffer_copyStringLen(CFL_BUFFERP buffer, CFL_STRP destStr, CFL_UINT32 len);
extern CFL_BOOL cfl_buffer_putStringLen(CFL_BUFFERP buffer, CFL_STRP value, CFL_UINT32 len);
extern CFL_BOOL cfl_buffer_putString(CFL_BUFFERP buffer, CFL_STRP value);
extern CFL_UINT32 cfl_buffer_getCharArrayLength(CFL_BUFFERP buffer);
extern char * cfl_buffer_getCharArray(CFL_BUFFERP buffer);
extern void cfl_buffer_copyCharArray(CFL_BUFFERP buffer, char *destStr);
extern void cfl_buffer_copyCharArrayLen(CFL_BUFFERP buffer, char *destStr, CFL_UINT32 len);
extern CFL_BOOL cfl_buffer_putCharArrayLen(CFL_BUFFERP buffer, const char *value, CFL_UINT32 len);
extern CFL_BOOL cfl_buffer_putCharArray(CFL_BUFFERP buffer, const char *value);
extern void cfl_buffer_getDate(CFL_BUFFERP buffer, CFL_DATEP date);
extern CFL_BOOL cfl_buffer_putDate(CFL_BUFFERP buffer, CFL_DATE value);
extern CFL_BOOL cfl_buffer_putDatePtr(CFL_BUFFERP buffer, CFL_DATEP value);
extern CFL_UINT8 *cfl_buffer_get(CFL_BUFFERP buffer, CFL_UINT32 size);
extern void cfl_buffer_copy(CFL_BUFFERP buffer, CFL_UINT8 *dest, CFL_UINT32 size);
extern CFL_BOOL cfl_buffer_put(CFL_BUFFERP buffer, void *value, CFL_UINT32 size);
extern CFL_BOOL cfl_buffer_putBuffer(CFL_BUFFERP buffer, CFL_BUFFERP other);
extern CFL_BOOL cfl_buffer_putBufferSize(CFL_BUFFERP buffer, CFL_BUFFERP other, CFL_UINT32 size);
extern CFL_UINT32 cfl_buffer_remaining(CFL_BUFFERP buffer);
extern CFL_BOOL cfl_buffer_haveEnough(CFL_BUFFERP buffer, CFL_UINT32 need);
extern CFL_BOOL cfl_buffer_putFormat(CFL_BUFFERP buffer, const char *format, ...);

#endif