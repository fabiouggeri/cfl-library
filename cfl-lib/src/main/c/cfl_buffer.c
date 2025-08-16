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
#include <stdio.h>
#include <string.h>

#include "cfl_buffer.h"
#include "cfl_str.h"
#include "cfl_mem.h"

#define BUFFER_INI_SIZE 8192

#define ENDIANNESS(i)    ((*(char*)&i) == 0 ? CFL_BIG_ENDIAN : CFL_LITTLE_ENDIAN)

#define PUT_BUFFER(t, b, v) \
         if (ensureCapacity(b, b->length + sizeof(t))) {\
            *((t *)&b->data[b->position]) = v;\
            b->position += sizeof(t);\
            if (b->position > b->length) {\
               b->length = b->position;\
            }\
         } else return CFL_FALSE

#define GET_BUFFER(t, b) \
         *((t *)&b->data[b->position]); \
         b->position += sizeof(t)

#define PEEK_BUFFER(t, b) \
         *((t *)&b->data[b->position]);\
         b->position += sizeof(t)

#define RETURN_BUFFER(t, b) \
         b->position += sizeof(t); \
         if (b->position > b->length) b->length = b->position; \
         return *((t *)&b->data[b->position - sizeof(t)])

#define PEEK_RETURN_BUFFER(t, b) \
         return *((t *)&b->data[b->position])

const int i = 1;

static CFL_BOOL ensureCapacity(CFL_BUFFERP buffer, CFL_UINT32 minCapacity) {
   if (minCapacity > buffer->capacity) {
      buffer->data = (CFL_UINT8 *) CFL_MEM_REALLOC(buffer->data, minCapacity * sizeof(CFL_UINT8));
      if (buffer->data != NULL) {
         buffer->capacity = minCapacity;
      } else {
         return CFL_FALSE;
      }
   }
   return CFL_TRUE;
}

static void bufferInit(CFL_BUFFERP buffer, CFL_UINT32 initialCapacity) {
	buffer->allocated = CFL_FALSE;
   buffer->capacity = initialCapacity > 0 ? initialCapacity : BUFFER_INI_SIZE;
   buffer->length = 0;
   buffer->position = 0;
   buffer->endian = ENDIANNESS(initialCapacity);
   buffer->data = (CFL_UINT8 *) CFL_MEM_ALLOC(buffer->capacity * sizeof(CFL_UINT8));
}

void cfl_buffer_init(CFL_BUFFERP buffer) {
   bufferInit(buffer, BUFFER_INI_SIZE);
}

CFL_BUFFERP cfl_buffer_new(void) {
   CFL_BUFFERP buffer = (CFL_BUFFERP) CFL_MEM_ALLOC(sizeof(CFL_BUFFER));
   if (buffer != NULL) {
      bufferInit(buffer, BUFFER_INI_SIZE);
      buffer->allocated = CFL_TRUE;
   }
   return buffer;
}

CFL_BUFFERP cfl_buffer_newCapacity(CFL_UINT32 initialCapacity) {
   CFL_BUFFERP buffer = (CFL_BUFFERP) CFL_MEM_ALLOC(sizeof(CFL_BUFFER));
   if (buffer != NULL) {
      bufferInit(buffer, initialCapacity);
      buffer->allocated = CFL_TRUE;
   }
   return buffer;
}

CFL_BUFFERP cfl_buffer_clone(CFL_BUFFERP other) {
   CFL_BUFFERP buffer = (CFL_BUFFERP) CFL_MEM_ALLOC(sizeof(CFL_BUFFER));
   if (buffer != NULL) {
      buffer->allocated = CFL_TRUE;
      buffer->capacity = other->length > 0 ? other->length : other->capacity;
      buffer->length = other->length;
      buffer->position = other->position;
      buffer->endian = other->endian;
      buffer->data = (CFL_UINT8 *) CFL_MEM_ALLOC(buffer->capacity * sizeof(CFL_UINT8));
      if (other->length > 0) {
         memcpy(buffer->data, other->data, other->length);
      }
   }
   return buffer;
}

void cfl_buffer_free(CFL_BUFFERP buffer) {
   if (buffer != NULL) {
      if (buffer->data) {
         CFL_MEM_FREE(buffer->data);
      }
      if (buffer->allocated) {
         CFL_MEM_FREE(buffer);
      }
   }
}

CFL_UINT8 *cfl_buffer_getDataPtr(const CFL_BUFFERP buffer) {
   return buffer->data;
}

CFL_UINT8 *cfl_buffer_positionPtr(CFL_BUFFERP buffer) {
   return &buffer->data[buffer->position];
}

void cfl_buffer_setBigEndian(CFL_BUFFERP buffer, CFL_BOOL bigEndian) {
   if (buffer != NULL) {
      buffer->endian = bigEndian ? CFL_BIG_ENDIAN : CFL_LITTLE_ENDIAN;
   }
}

CFL_BOOL cfl_buffer_isBigEndian(const CFL_BUFFERP buffer) {
   if (buffer != NULL) {
      return buffer->endian == CFL_BIG_ENDIAN;
   }
   return CFL_FALSE;
}

void cfl_buffer_flip(CFL_BUFFERP buffer) {
   buffer->length = buffer->position;
   buffer->position = 0;
}

CFL_UINT32 cfl_buffer_length(const CFL_BUFFERP buffer) {
   return buffer->length;
}

CFL_BOOL cfl_buffer_setLength(CFL_BUFFERP buffer, CFL_UINT32 newLen) {
   if (! ensureCapacity(buffer, newLen)) {
      return CFL_FALSE;
   }
   buffer->length = newLen;
   if (buffer->position > newLen) {
      buffer->position = newLen;
   }
   return CFL_TRUE;
}

void cfl_buffer_reset(CFL_BUFFERP buffer) {
   buffer->length = 0;
   buffer->position = 0;
}

CFL_UINT32 cfl_buffer_position(const CFL_BUFFERP buffer) {
   return buffer->position;
}

void cfl_buffer_setPosition(CFL_BUFFERP buffer, CFL_UINT32 newPos) {

   buffer->position = newPos;
   if (buffer->position > buffer->length) {
      buffer->length = buffer->position;
      ensureCapacity(buffer, buffer->length);
   }
}

void cfl_buffer_skip(CFL_BUFFERP buffer, CFL_UINT32 skip) {
   buffer->position += skip;
   if (buffer->position > buffer->length) {
      buffer->length = buffer->position;
      ensureCapacity(buffer, buffer->length);
   }
}

void cfl_buffer_rewind(CFL_BUFFERP buffer) {
   buffer->position = 0;
}

void cfl_buffer_compact(CFL_BUFFERP buffer) {
   if (buffer->position > 0) {
      if (buffer->length > buffer->position) {
         CFL_UINT32 len = buffer->length - buffer->position;
         memcpy((void *)&buffer->data[0], (void *)&buffer->data[buffer->position], len);
         buffer->position = 0;
         buffer->length = len;
      } else {
         buffer->position = 0;
         buffer->length = 0;
      }
   }
}

CFL_UINT32 cfl_buffer_capacity(const CFL_BUFFERP buffer) {
   return buffer->capacity;
}

CFL_BOOL cfl_buffer_setCapacity(CFL_BUFFERP buffer, CFL_UINT32 newCapacity) {
   if (newCapacity > 0) {
      if (newCapacity > buffer->capacity) {
         return ensureCapacity(buffer, newCapacity);
      } else {
         buffer->data = CFL_MEM_REALLOC(buffer->data, newCapacity);
         if (buffer->data != NULL) {
            buffer->capacity = newCapacity;
            if (buffer->length > newCapacity) {
               buffer->length = newCapacity;
               if (buffer->position > newCapacity) {
                  buffer->position = newCapacity;
               }
            }
            return CFL_TRUE;
         }
      }
   }
   return CFL_FALSE;
}

CFL_BOOL cfl_buffer_putBoolean(CFL_BUFFERP buffer, CFL_BOOL value) {
   PUT_BUFFER(CFL_BOOL, buffer, value);
   return CFL_TRUE;
}

CFL_BOOL cfl_buffer_getBoolean(CFL_BUFFERP buffer) {
   RETURN_BUFFER(CFL_BOOL, buffer);
}

CFL_BOOL cfl_buffer_peekBoolean(CFL_BUFFERP buffer) {
   PEEK_RETURN_BUFFER(CFL_BOOL, buffer);
}

CFL_BOOL cfl_buffer_putInt8(CFL_BUFFERP buffer, CFL_INT8 value) {
   PUT_BUFFER(CFL_INT8, buffer, value);
   return CFL_TRUE;
}

CFL_INT8 cfl_buffer_getInt8(CFL_BUFFERP buffer) {
   RETURN_BUFFER(CFL_INT8, buffer);
}

CFL_INT8 cfl_buffer_peekInt8(CFL_BUFFERP buffer) {
   PEEK_RETURN_BUFFER(CFL_INT8, buffer);
}

CFL_BOOL cfl_buffer_putInt16(CFL_BUFFERP buffer, CFL_INT16 value) {
   PUT_BUFFER(CFL_INT16, buffer, value);
   return CFL_TRUE;
}

CFL_INT16 cfl_buffer_getInt16(CFL_BUFFERP buffer) {
   RETURN_BUFFER(CFL_INT16, buffer);
}

CFL_INT16 cfl_buffer_peekInt16(CFL_BUFFERP buffer) {
   PEEK_RETURN_BUFFER(CFL_INT16, buffer);
}

CFL_BOOL cfl_buffer_putInt32(CFL_BUFFERP buffer, CFL_INT32 value) {
   PUT_BUFFER(CFL_INT32, buffer, value);
   return CFL_TRUE;
}

CFL_INT32 cfl_buffer_getInt32(CFL_BUFFERP buffer) {
   RETURN_BUFFER(CFL_INT32, buffer);
}

CFL_INT32 cfl_buffer_peekInt32(CFL_BUFFERP buffer) {
   PEEK_RETURN_BUFFER(CFL_INT32, buffer);
}

CFL_BOOL cfl_buffer_putInt64(CFL_BUFFERP buffer, CFL_INT64 value) {
   PUT_BUFFER(CFL_INT64, buffer, value);
   return CFL_TRUE;
}

CFL_INT64 cfl_buffer_getInt64(CFL_BUFFERP buffer) {
   RETURN_BUFFER(CFL_INT64, buffer);
}

CFL_INT64 cfl_buffer_peekInt64(CFL_BUFFERP buffer) {
   PEEK_RETURN_BUFFER(CFL_INT64, buffer);
}

CFL_BOOL cfl_buffer_putUInt8(CFL_BUFFERP buffer, CFL_UINT8 value) {
   PUT_BUFFER(CFL_UINT8, buffer, value);
   return CFL_TRUE;
}

CFL_UINT8 cfl_buffer_getUInt8(CFL_BUFFERP buffer) {
   RETURN_BUFFER(CFL_UINT8, buffer);
}

CFL_UINT8 cfl_buffer_peekUInt8(CFL_BUFFERP buffer) {
   PEEK_RETURN_BUFFER(CFL_UINT8, buffer);
}

CFL_BOOL cfl_buffer_putUInt16(CFL_BUFFERP buffer, CFL_UINT16 value) {
   PUT_BUFFER(CFL_UINT16, buffer, value);
   return CFL_TRUE;
}

CFL_UINT16 cfl_buffer_getUInt16(CFL_BUFFERP buffer) {
   RETURN_BUFFER(CFL_UINT16, buffer);
}

CFL_UINT16 cfl_buffer_peekUInt16(CFL_BUFFERP buffer) {
   PEEK_RETURN_BUFFER(CFL_UINT16, buffer);
}

CFL_BOOL cfl_buffer_putUInt32(CFL_BUFFERP buffer, CFL_UINT32 value) {
   PUT_BUFFER(CFL_UINT32, buffer, value);
   return CFL_TRUE;
}

CFL_UINT32 cfl_buffer_getUInt32(CFL_BUFFERP buffer) {
   RETURN_BUFFER(CFL_UINT32, buffer);
}

CFL_UINT32 cfl_buffer_peekUInt32(CFL_BUFFERP buffer) {
   PEEK_RETURN_BUFFER(CFL_UINT32, buffer);
}

CFL_BOOL cfl_buffer_putUInt64(CFL_BUFFERP buffer, CFL_UINT64 value) {
   PUT_BUFFER(CFL_UINT64, buffer, value);
   return CFL_TRUE;
}

CFL_UINT64 cfl_buffer_getUInt64(CFL_BUFFERP buffer) {
   RETURN_BUFFER(CFL_UINT64, buffer);
}

CFL_UINT64 cfl_buffer_peekUInt64(CFL_BUFFERP buffer) {
   PEEK_RETURN_BUFFER(CFL_UINT64, buffer);
}

CFL_BOOL cfl_buffer_putFloat(CFL_BUFFERP buffer, float value) {
   PUT_BUFFER(float, buffer, value);
   return CFL_TRUE;
}

float cfl_buffer_getFloat(CFL_BUFFERP buffer) {
   RETURN_BUFFER(float, buffer);
}

float cfl_buffer_peekFloat(CFL_BUFFERP buffer) {
   PEEK_RETURN_BUFFER(float, buffer);
}

CFL_BOOL cfl_buffer_putDouble(CFL_BUFFERP buffer, double value) {
   PUT_BUFFER(double, buffer, value);
   return CFL_TRUE;
}

double cfl_buffer_getDouble(CFL_BUFFERP buffer) {
   RETURN_BUFFER(double, buffer);
}

double cfl_buffer_peekDouble(CFL_BUFFERP buffer) {
   PEEK_RETURN_BUFFER(double, buffer);
}

CFL_STRP cfl_buffer_getString(CFL_BUFFERP buffer) {
   CFL_STRP str;
   CFL_UINT32 len;

   len = GET_BUFFER(CFL_UINT32, buffer);
   if (len > 0) {
      str = cfl_str_newBufferLen((const char *)&buffer->data[buffer->position], len);
      buffer->position += len;
   } else {
      str = cfl_str_new(16);
   }
   return str;
}

CFL_UINT32 cfl_buffer_getStringLength(CFL_BUFFERP buffer) {
   return *((CFL_UINT32 *)&buffer->data[buffer->position]);
}

void cfl_buffer_copyString(CFL_BUFFERP buffer, CFL_STRP destStr) {
   CFL_UINT32 len;

   len = GET_BUFFER(CFL_UINT32, buffer);
   if (len > 0) {
      cfl_str_setValueLen(destStr, (const char *)&buffer->data[buffer->position], len);
      buffer->position += len;
   }
}

void cfl_buffer_copyStringLen(CFL_BUFFERP buffer, CFL_STRP destStr, CFL_UINT32 len) {
   CFL_UINT32 strLen;

   strLen = GET_BUFFER(CFL_UINT32, buffer);
   if (len > strLen) {
      len = strLen;
   }
   if (len > 0) {
      cfl_str_setValueLen(destStr, (const char *)&buffer->data[buffer->position], len);
      buffer->position += len;
   }
}

CFL_BOOL cfl_buffer_putString(CFL_BUFFERP buffer, CFL_STRP str) {
   CFL_UINT32 len = cfl_str_length(str);

   PUT_BUFFER(CFL_UINT32, buffer, len);
   if (len > 0) {
      if (! ensureCapacity(buffer, buffer->length + len)) {
         return CFL_FALSE;
      }
      memcpy((void *)&buffer->data[buffer->position], (void *)cfl_str_getPtr(str), len);
      buffer->position += len;
      if (buffer->position > buffer->length) {
         buffer->length = buffer->position;
      }
   }
   return CFL_TRUE;
}

CFL_BOOL cfl_buffer_putStringLen(CFL_BUFFERP buffer, CFL_STRP str, CFL_UINT32 len) {
   PUT_BUFFER(CFL_UINT32, buffer, len);
   if (len > 0) {
      CFL_UINT32 strLen = cfl_str_length(str);
      if (strLen > len) {
         strLen = len;
      }
      if (! ensureCapacity(buffer, buffer->length + len)) {
         return CFL_FALSE;
      }
      memcpy((void *)&buffer->data[buffer->position], (void *)cfl_str_getPtr(str), strLen);
      while (strLen < len) {
         buffer->data[buffer->position + strLen] = ' ';
         ++strLen;
      }
      buffer->position += strLen;
      if (buffer->position > buffer->length) {
         buffer->length = buffer->position;
      }
   }
   return CFL_TRUE;
}

char * cfl_buffer_getCharArray(CFL_BUFFERP buffer) {
   char *str;
   CFL_UINT32 len;

   len = GET_BUFFER(CFL_UINT32, buffer);
   str = (char *) CFL_MEM_ALLOC((len + 1) * sizeof(char));
   if (len > 0) {
      memcpy((void *)str, (void *)&buffer->data[buffer->position], len * sizeof(char));
      buffer->position += len;
   }
   str[len] = 0;
   return str;
}

CFL_UINT32 cfl_buffer_getCharArrayLength(CFL_BUFFERP buffer) {
   return *((CFL_UINT32 *)&buffer->data[buffer->position]);
}


void cfl_buffer_copyCharArray(CFL_BUFFERP buffer, char *destStr) {
   CFL_UINT32 len;

   len = GET_BUFFER(CFL_UINT32, buffer);
   if (len > 0) {
      memcpy((void *)destStr, (void *)&buffer->data[buffer->position], len * sizeof(char));
      buffer->position += len;
   }
   destStr[len] = 0;
}

void cfl_buffer_copyCharArrayLen(CFL_BUFFERP buffer, char *destStr, CFL_UINT32 len) {
   CFL_UINT32 numBytes;

   numBytes = GET_BUFFER(CFL_UINT32, buffer);
   if (numBytes > 0) {
      CFL_UINT32 maxBytes = len * sizeof(char);
      if (maxBytes > numBytes) {
         maxBytes = numBytes;
      }
      memcpy((void *)destStr, (void *)&buffer->data[buffer->position], maxBytes);
      buffer->position += maxBytes;
   }
}

CFL_BOOL cfl_buffer_putCharArray(CFL_BUFFERP buffer, const char *value) {
   CFL_UINT32 len = (CFL_UINT32) strlen(value) * sizeof(char);

   PUT_BUFFER(CFL_UINT32, buffer, len);
   if (len > 0) {
      if (! ensureCapacity(buffer, buffer->length + len)) {
         return CFL_FALSE;
      }
      memcpy((void *)&buffer->data[buffer->position], (void *)value, len);
      buffer->position += len;
      if (buffer->position > buffer->length) {
         buffer->length = buffer->position;
      }
   }
   return CFL_TRUE;
}

CFL_BOOL cfl_buffer_putCharArrayLen(CFL_BUFFERP buffer, const char *value, CFL_UINT32 len) {
   PUT_BUFFER(CFL_UINT32, buffer, len);
   if (len > 0) {
      if (! ensureCapacity(buffer, buffer->length + len)) {
         return CFL_FALSE;
      }
      memcpy((void *)&buffer->data[buffer->position], (void *)value, len * sizeof(char));
      buffer->position += len;
      if (buffer->position > buffer->length) {
         buffer->length = buffer->position;
      }
   }
   return CFL_TRUE;
}

void cfl_buffer_getDate(CFL_BUFFERP buffer, CFL_DATEP date) {
   CFL_UINT16 year;
   CFL_UINT8 month;
   CFL_UINT8 day;
   year = GET_BUFFER(CFL_UINT16, buffer);
   month = GET_BUFFER(CFL_UINT8, buffer);
   day = GET_BUFFER(CFL_UINT8, buffer);
   cfl_date_setDate(date, year, month, day);
}

CFL_BOOL cfl_buffer_putDate(CFL_BUFFERP buffer, CFL_DATE value) {
   PUT_BUFFER(CFL_UINT16, buffer, cfl_date_getYear(&value));
   PUT_BUFFER(CFL_UINT8, buffer, cfl_date_getMonth(&value));
   PUT_BUFFER(CFL_UINT8, buffer, cfl_date_getDay(&value));
   return CFL_TRUE;
}

CFL_BOOL cfl_buffer_putDatePtr(CFL_BUFFERP buffer, CFL_DATEP value) {
   PUT_BUFFER(CFL_UINT16, buffer, cfl_date_getYear(value));
   PUT_BUFFER(CFL_UINT8, buffer, cfl_date_getMonth(value));
   PUT_BUFFER(CFL_UINT8, buffer, cfl_date_getDay(value));
   return CFL_TRUE;
}

CFL_UINT8 *cfl_buffer_get(CFL_BUFFERP buffer, CFL_UINT32 size) {
   void *res = CFL_MEM_ALLOC(size);

   memcpy(res, (CFL_UINT8 *)&buffer->data[buffer->position], size);
   buffer->position += size;
   return res;
}

void cfl_buffer_copy(CFL_BUFFERP buffer, CFL_UINT8 *dest, CFL_UINT32 size) {
   memcpy((CFL_UINT8 *)dest, (CFL_UINT8 *)&buffer->data[buffer->position], size);
   buffer->position += size;
}

CFL_BOOL cfl_buffer_put(CFL_BUFFERP buffer, void *value, CFL_UINT32 size) {
   if (! ensureCapacity(buffer, buffer->length + size)) {
      return CFL_FALSE;
   }
   memcpy((void *)&buffer->data[buffer->position], (void *)value, size);
   buffer->position += size;
   if (buffer->position > buffer->length) {
      buffer->length = buffer->position;
   }
   return CFL_TRUE;
}

CFL_BOOL cfl_buffer_putBuffer(CFL_BUFFERP buffer, CFL_BUFFERP other) {
   CFL_UINT32 otherSize = cfl_buffer_remaining(other);
   if (otherSize > 0) {
      if (! ensureCapacity(buffer, buffer->length + otherSize)) {
         return CFL_FALSE;
      }
      memcpy((void *)&buffer->data[buffer->position], (void *)&other->data[other->position], otherSize);
      buffer->position += otherSize;
      other->position += otherSize;
      if (buffer->position > buffer->length) {
         buffer->length = buffer->position;
      }
   }
   return CFL_TRUE;
}

CFL_BOOL cfl_buffer_putBufferSize(CFL_BUFFERP buffer, CFL_BUFFERP other, CFL_UINT32 size) {
   CFL_UINT32 otherSize = cfl_buffer_remaining(other);
   if (size > 0 && size < otherSize) {
      otherSize = size;
   }
   if (otherSize > 0) {
      if (! ensureCapacity(buffer, buffer->length + otherSize)) {
         return CFL_FALSE;
      }
      memcpy((void *)&buffer->data[buffer->position], (void *)&other->data[other->position], otherSize);
      buffer->position += otherSize;
      other->position += otherSize;
      if (buffer->position > buffer->length) {
         buffer->length = buffer->position;
      }
   }
   return CFL_TRUE;
}

CFL_UINT32 cfl_buffer_remaining(const CFL_BUFFERP buffer) {
   if (buffer->length > buffer->position) {
      return buffer->length - buffer->position;
   }
   return 0;
}

CFL_BOOL cfl_buffer_haveEnough(const CFL_BUFFERP buffer, CFL_UINT32 need) {
   return buffer->length > buffer->position && need <= (buffer->length - buffer->position);
}

CFL_BOOL cfl_buffer_putFormatArgs(CFL_BUFFERP buffer, const char * format, va_list varArgs) {
   int iLen;

   iLen = vsnprintf(NULL, 0, format, varArgs);
   if (iLen > 0) {
      PUT_BUFFER(CFL_UINT32, buffer, (CFL_UINT32) iLen);
      if (! ensureCapacity(buffer, buffer->length + iLen)) {
         return CFL_FALSE;
      }
      iLen = vsnprintf((char *)&buffer->data[buffer->position], iLen, format, varArgs);
      buffer->position += iLen;
      if (buffer->position > buffer->length) {
         buffer->length = buffer->position;
      }
   } else if (iLen == 0) {
      PUT_BUFFER(CFL_UINT32, buffer, 0);
   }
   return CFL_TRUE;
}

CFL_BOOL cfl_buffer_putFormat(CFL_BUFFERP buffer, const char * format, ...) {
   CFL_BOOL bSuccess;
   va_list varArgs;

   va_start(varArgs, format);
   bSuccess = cfl_buffer_putFormatArgs(buffer, format, varArgs);
   va_end(varArgs);
   return bSuccess;
}

CFL_BOOL cfl_buffer_moveTo(CFL_BUFFERP fromBuffer, CFL_BUFFERP toBuffer) {
   if (fromBuffer == NULL || toBuffer == NULL) {
      return CFL_FALSE;
   }
   if (toBuffer->data != NULL) {
      CFL_MEM_FREE(toBuffer->data);
   }
	toBuffer->data      = fromBuffer->data;
	toBuffer->length    = fromBuffer->length;
	toBuffer->position  = fromBuffer->position;
	toBuffer->capacity  = fromBuffer->capacity;
   toBuffer->endian    = fromBuffer->endian;
	fromBuffer->data      = NULL;
	fromBuffer->length    = 0;
	fromBuffer->position  = 0;
	fromBuffer->capacity  = 0;
   return CFL_TRUE;
}
