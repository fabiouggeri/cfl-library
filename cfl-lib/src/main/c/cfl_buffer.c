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

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cfl_buffer.h"
#include "cfl_mem.h"
#include "cfl_str.h"

#define BUFFER_INI_SIZE 8192

static const int _endian_check = 1;
#define IS_LITTLE_ENDIAN (*(char *)&_endian_check == 1)
#define HOST_ENDIAN (IS_LITTLE_ENDIAN ? CFL_LITTLE_ENDIAN : CFL_BIG_ENDIAN)

static void swap_bytes(void *p, size_t n) {
   unsigned char *lo = (unsigned char *)p;
   unsigned char *hi = (unsigned char *)p + n - 1;
   while (lo < hi) {
      unsigned char tmp = *lo;
      *lo++ = *hi;
      *hi-- = tmp;
   }
}

static CFL_BOOL ensureCapacity(CFL_BUFFERP buffer, CFL_UINT32 minCapacity) {
   if (minCapacity > buffer->capacity) {
      CFL_UINT32 newCapacity;
      CFL_UINT8 *newData;

      // Check for overflow
      // newCapacity = (capacity / 2) + 1 + minCapacity
      if (minCapacity > CFL_UINT32_MAX - (buffer->capacity >> 1) - 1) {
         return CFL_FALSE;
      }

      newCapacity = (buffer->capacity >> 1) + 1 + minCapacity;
      newData = (CFL_UINT8 *)CFL_MEM_REALLOC(buffer->data, newCapacity * sizeof(CFL_UINT8));
      if (newData != NULL) {
         buffer->data = newData;
         buffer->capacity = newCapacity;
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
   buffer->endian = HOST_ENDIAN;
   buffer->data = (CFL_UINT8 *)CFL_MEM_ALLOC(buffer->capacity * sizeof(CFL_UINT8));
}

static CFL_BOOL buffer_put_val(CFL_BUFFERP buffer, const void *val, size_t size) {
   if (!ensureCapacity(buffer, buffer->position + (CFL_UINT32)size)) {
      return CFL_FALSE;
   }
   if (buffer->endian != HOST_ENDIAN && size > 1) {
      CFL_UINT8 tmp[8];
      if (size <= 8) {
         memcpy(tmp, val, size);
         swap_bytes(tmp, size);
         memcpy(&buffer->data[buffer->position], tmp, size);
      } else {
         return CFL_FALSE;
      }
   } else {
      memcpy(&buffer->data[buffer->position], val, size);
   }
   buffer->position += (CFL_UINT32)size;
   if (buffer->position > buffer->length) {
      buffer->length = buffer->position;
   }
   return CFL_TRUE;
}

static void buffer_read_val(CFL_BUFFERP buffer, void *val, size_t size) {
   if (buffer->endian != HOST_ENDIAN && size > 1) {
      CFL_UINT8 tmp[8];
      if (size <= 8) {
         memcpy(tmp, &buffer->data[buffer->position], size);
         swap_bytes(tmp, size);
         memcpy(val, tmp, size);
      }
   } else {
      memcpy(val, &buffer->data[buffer->position], size);
   }
   buffer->position += (CFL_UINT32)size;
}

static void buffer_consume_val(CFL_BUFFERP buffer, void *val, size_t size) {
   buffer_read_val(buffer, val, size);
   if (buffer->position > buffer->length) {
      buffer->length = buffer->position;
   }
}

static void buffer_peek_val(CFL_BUFFERP buffer, void *val, size_t size) {
   if (buffer->endian != HOST_ENDIAN && size > 1) {
      CFL_UINT8 tmp[8];
      if (size <= 8) {
         memcpy(tmp, &buffer->data[buffer->position], size);
         swap_bytes(tmp, size);
         memcpy(val, tmp, size);
      }
   } else {
      memcpy(val, &buffer->data[buffer->position], size);
   }
}

void cfl_buffer_init(CFL_BUFFERP buffer) {
   bufferInit(buffer, BUFFER_INI_SIZE);
}

CFL_BUFFERP cfl_buffer_new(void) {
   CFL_BUFFERP buffer = (CFL_BUFFERP)CFL_MEM_ALLOC(sizeof(CFL_BUFFER));
   if (buffer != NULL) {
      bufferInit(buffer, BUFFER_INI_SIZE);
      buffer->allocated = CFL_TRUE;
   }
   return buffer;
}

CFL_BUFFERP cfl_buffer_newCapacity(CFL_UINT32 initialCapacity) {
   CFL_BUFFERP buffer = (CFL_BUFFERP)CFL_MEM_ALLOC(sizeof(CFL_BUFFER));
   if (buffer != NULL) {
      bufferInit(buffer, initialCapacity);
      buffer->allocated = CFL_TRUE;
   }
   return buffer;
}

CFL_BUFFERP cfl_buffer_clone(CFL_BUFFERP other) {
   CFL_BUFFERP buffer = (CFL_BUFFERP)CFL_MEM_ALLOC(sizeof(CFL_BUFFER));
   if (buffer != NULL) {
      buffer->allocated = CFL_TRUE;
      buffer->capacity = other->length > 0 ? other->length : other->capacity;
      buffer->length = other->length;
      buffer->position = other->position;
      buffer->endian = other->endian;
      buffer->data = (CFL_UINT8 *)CFL_MEM_ALLOC(buffer->capacity * sizeof(CFL_UINT8));
      if (buffer->data == NULL) {
         CFL_MEM_FREE(buffer);
         return NULL;
      }
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
   if (!ensureCapacity(buffer, newLen)) {
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
   // check overflow
   if (buffer->position > CFL_UINT32_MAX - skip) {
      return;
   }
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
   return buffer_put_val(buffer, &value, sizeof(value));
}

CFL_BOOL cfl_buffer_getBoolean(CFL_BUFFERP buffer) {
   CFL_BOOL value = CFL_FALSE;
   buffer_consume_val(buffer, &value, sizeof(value));
   return value;
}

CFL_BOOL cfl_buffer_peekBoolean(CFL_BUFFERP buffer) {
   CFL_BOOL value = CFL_FALSE;
   buffer_peek_val(buffer, &value, sizeof(value));
   return value;
}

CFL_BOOL cfl_buffer_putInt8(CFL_BUFFERP buffer, CFL_INT8 value) {
   return buffer_put_val(buffer, &value, sizeof(value));
}

CFL_INT8 cfl_buffer_getInt8(CFL_BUFFERP buffer) {
   CFL_INT8 value = 0;
   buffer_consume_val(buffer, &value, sizeof(value));
   return value;
}

CFL_INT8 cfl_buffer_peekInt8(CFL_BUFFERP buffer) {
   CFL_INT8 value = 0;
   buffer_peek_val(buffer, &value, sizeof(value));
   return value;
}

CFL_BOOL cfl_buffer_putInt16(CFL_BUFFERP buffer, CFL_INT16 value) {
   return buffer_put_val(buffer, &value, sizeof(value));
}

CFL_INT16 cfl_buffer_getInt16(CFL_BUFFERP buffer) {
   CFL_INT16 value = 0;
   buffer_consume_val(buffer, &value, sizeof(value));
   return value;
}

CFL_INT16 cfl_buffer_peekInt16(CFL_BUFFERP buffer) {
   CFL_INT16 value = 0;
   buffer_peek_val(buffer, &value, sizeof(value));
   return value;
}

CFL_BOOL cfl_buffer_putInt32(CFL_BUFFERP buffer, CFL_INT32 value) {
   return buffer_put_val(buffer, &value, sizeof(value));
}

CFL_INT32 cfl_buffer_getInt32(CFL_BUFFERP buffer) {
   CFL_INT32 value = 0;
   buffer_consume_val(buffer, &value, sizeof(value));
   return value;
}

CFL_INT32 cfl_buffer_peekInt32(CFL_BUFFERP buffer) {
   CFL_INT32 value = 0;
   buffer_peek_val(buffer, &value, sizeof(value));
   return value;
}

CFL_BOOL cfl_buffer_putInt64(CFL_BUFFERP buffer, CFL_INT64 value) {
   return buffer_put_val(buffer, &value, sizeof(value));
}

CFL_INT64 cfl_buffer_getInt64(CFL_BUFFERP buffer) {
   CFL_INT64 value = 0;
   buffer_consume_val(buffer, &value, sizeof(value));
   return value;
}

CFL_INT64 cfl_buffer_peekInt64(CFL_BUFFERP buffer) {
   CFL_INT64 value = 0;
   buffer_peek_val(buffer, &value, sizeof(value));
   return value;
}

CFL_BOOL cfl_buffer_putUInt8(CFL_BUFFERP buffer, CFL_UINT8 value) {
   return buffer_put_val(buffer, &value, sizeof(value));
}

CFL_UINT8 cfl_buffer_getUInt8(CFL_BUFFERP buffer) {
   CFL_UINT8 value = 0;
   buffer_consume_val(buffer, &value, sizeof(value));
   return value;
}

CFL_UINT8 cfl_buffer_peekUInt8(CFL_BUFFERP buffer) {
   CFL_UINT8 value = 0;
   buffer_peek_val(buffer, &value, sizeof(value));
   return value;
}

CFL_BOOL cfl_buffer_putUInt16(CFL_BUFFERP buffer, CFL_UINT16 value) {
   return buffer_put_val(buffer, &value, sizeof(value));
}

CFL_UINT16 cfl_buffer_getUInt16(CFL_BUFFERP buffer) {
   CFL_UINT16 value = 0;
   buffer_consume_val(buffer, &value, sizeof(value));
   return value;
}

CFL_UINT16 cfl_buffer_peekUInt16(CFL_BUFFERP buffer) {
   CFL_UINT16 value = 0;
   buffer_peek_val(buffer, &value, sizeof(value));
   return value;
}

CFL_BOOL cfl_buffer_putUInt32(CFL_BUFFERP buffer, CFL_UINT32 value) {
   return buffer_put_val(buffer, &value, sizeof(value));
}

CFL_UINT32 cfl_buffer_getUInt32(CFL_BUFFERP buffer) {
   CFL_UINT32 value = 0;
   buffer_consume_val(buffer, &value, sizeof(value));
   return value;
}

CFL_UINT32 cfl_buffer_peekUInt32(CFL_BUFFERP buffer) {
   CFL_UINT32 value = 0;
   buffer_peek_val(buffer, &value, sizeof(value));
   return value;
}

CFL_BOOL cfl_buffer_putUInt64(CFL_BUFFERP buffer, CFL_UINT64 value) {
   return buffer_put_val(buffer, &value, sizeof(value));
}

CFL_UINT64 cfl_buffer_getUInt64(CFL_BUFFERP buffer) {
   CFL_UINT64 value = 0;
   buffer_consume_val(buffer, &value, sizeof(value));
   return value;
}

CFL_UINT64 cfl_buffer_peekUInt64(CFL_BUFFERP buffer) {
   CFL_UINT64 value = 0;
   buffer_peek_val(buffer, &value, sizeof(value));
   return value;
}

CFL_BOOL cfl_buffer_putFloat(CFL_BUFFERP buffer, float value) {
   return buffer_put_val(buffer, &value, sizeof(value));
}

float cfl_buffer_getFloat(CFL_BUFFERP buffer) {
   float value = 0.0f;
   buffer_consume_val(buffer, &value, sizeof(value));
   return value;
}

float cfl_buffer_peekFloat(CFL_BUFFERP buffer) {
   float value = 0.0f;
   buffer_peek_val(buffer, &value, sizeof(value));
   return value;
}

CFL_BOOL cfl_buffer_putDouble(CFL_BUFFERP buffer, double value) {
   return buffer_put_val(buffer, &value, sizeof(value));
}

double cfl_buffer_getDouble(CFL_BUFFERP buffer) {
   double value = 0.0;
   buffer_consume_val(buffer, &value, sizeof(value));
   return value;
}

double cfl_buffer_peekDouble(CFL_BUFFERP buffer) {
   double value = 0.0;
   buffer_peek_val(buffer, &value, sizeof(value));
   return value;
}

CFL_STRP cfl_buffer_getString(CFL_BUFFERP buffer) {
   CFL_STRP str;
   CFL_UINT32 len;

   buffer_read_val(buffer, &len, sizeof(len));
   if (len > 0) {
      str = cfl_str_newBufferLen((const char *)&buffer->data[buffer->position], len);
      buffer->position += len;
   } else {
      str = cfl_str_new(16);
   }
   return str;
}

CFL_UINT32 cfl_buffer_getStringLength(CFL_BUFFERP buffer) {
   // Use peek to get length without advancing
   CFL_UINT32 len;
   buffer_peek_val(buffer, &len, sizeof(len));
   return len;
}

void cfl_buffer_copyString(CFL_BUFFERP buffer, CFL_STRP destStr) {
   CFL_UINT32 len;

   buffer_read_val(buffer, &len, sizeof(len));
   if (len > 0) {
      cfl_str_setValueLen(destStr, (const char *)&buffer->data[buffer->position], len);
      buffer->position += len;
   }
}

void cfl_buffer_copyStringLen(CFL_BUFFERP buffer, CFL_STRP destStr, CFL_UINT32 len) {
   CFL_UINT32 strLen;

   buffer_read_val(buffer, &strLen, sizeof(strLen));
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

   if (!buffer_put_val(buffer, &len, sizeof(len))) {
      return CFL_FALSE;
   }
   if (len > 0) {
      if (!ensureCapacity(buffer, buffer->position + len)) {
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
   if (!buffer_put_val(buffer, &len, sizeof(len))) {
      return CFL_FALSE;
   }
   if (len > 0) {
      CFL_UINT32 strLen = cfl_str_length(str);
      if (strLen > len) {
         strLen = len;
      }
      if (!ensureCapacity(buffer, buffer->position + len)) {
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

char *cfl_buffer_getCharArray(CFL_BUFFERP buffer) {
   char *str;
   CFL_UINT32 len;

   buffer_read_val(buffer, &len, sizeof(len));
   str = (char *)CFL_MEM_ALLOC((len + 1) * sizeof(char));
   if (str == NULL) {
      return NULL;
   }
   if (len > 0) {
      memcpy((void *)str, (void *)&buffer->data[buffer->position], len * sizeof(char));
      buffer->position += len;
   }
   str[len] = 0;
   return str;
}

CFL_UINT32 cfl_buffer_getCharArrayLength(CFL_BUFFERP buffer) {
   CFL_UINT32 len;
   buffer_peek_val(buffer, &len, sizeof(len));
   return len;
}

void cfl_buffer_copyCharArray(CFL_BUFFERP buffer, char *destStr) {
   CFL_UINT32 len;

   buffer_read_val(buffer, &len, sizeof(len));
   if (len > 0) {
      memcpy((void *)destStr, (void *)&buffer->data[buffer->position], len * sizeof(char));
      buffer->position += len;
   }
   destStr[len] = 0;
}

void cfl_buffer_copyCharArrayLen(CFL_BUFFERP buffer, char *destStr, CFL_UINT32 len) {
   CFL_UINT32 numBytes;

   buffer_read_val(buffer, &numBytes, sizeof(numBytes));
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
   CFL_UINT32 len = (CFL_UINT32)strlen(value) * sizeof(char);

   if (!buffer_put_val(buffer, &len, sizeof(len))) {
      return CFL_FALSE;
   }
   if (len > 0) {
      if (!ensureCapacity(buffer, buffer->position + len)) {
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
   if (!buffer_put_val(buffer, &len, sizeof(len))) {
      return CFL_FALSE;
   }
   if (len > 0) {
      if (!ensureCapacity(buffer, buffer->position + len)) {
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
   buffer_read_val(buffer, &year, sizeof(year));
   buffer_read_val(buffer, &month, sizeof(month));
   buffer_read_val(buffer, &day, sizeof(day));
   cfl_date_setDate(date, year, month, day);
}

CFL_BOOL cfl_buffer_putDate(CFL_BUFFERP buffer, CFL_DATE value) {
   CFL_UINT16 year = cfl_date_getYear(&value);
   CFL_UINT8 month = cfl_date_getMonth(&value);
   CFL_UINT8 day = cfl_date_getDay(&value);

   if (!buffer_put_val(buffer, &year, sizeof(year)))
      return CFL_FALSE;
   if (!buffer_put_val(buffer, &month, sizeof(month)))
      return CFL_FALSE;
   if (!buffer_put_val(buffer, &day, sizeof(day)))
      return CFL_FALSE;
   return CFL_TRUE;
}

CFL_BOOL cfl_buffer_putDatePtr(CFL_BUFFERP buffer, CFL_DATEP value) {
   CFL_UINT16 year = cfl_date_getYear(value);
   CFL_UINT8 month = cfl_date_getMonth(value);
   CFL_UINT8 day = cfl_date_getDay(value);

   if (!buffer_put_val(buffer, &year, sizeof(year)))
      return CFL_FALSE;
   if (!buffer_put_val(buffer, &month, sizeof(month)))
      return CFL_FALSE;
   if (!buffer_put_val(buffer, &day, sizeof(day)))
      return CFL_FALSE;
   return CFL_TRUE;
}

CFL_UINT8 *cfl_buffer_get(CFL_BUFFERP buffer, CFL_UINT32 size) {
   void *res = CFL_MEM_ALLOC(size);
   if (res == NULL)
      return NULL; // Safety

   memcpy(res, (CFL_UINT8 *)&buffer->data[buffer->position], size);
   buffer->position += size;
   return res;
}

void cfl_buffer_copy(CFL_BUFFERP buffer, CFL_UINT8 *dest, CFL_UINT32 size) {
   memcpy((CFL_UINT8 *)dest, (CFL_UINT8 *)&buffer->data[buffer->position], size);
   buffer->position += size;
}

CFL_BOOL cfl_buffer_put(CFL_BUFFERP buffer, void *value, CFL_UINT32 size) {
   if (!ensureCapacity(buffer, buffer->position + size)) {
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
      if (!ensureCapacity(buffer, buffer->position + otherSize)) {
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
      if (!ensureCapacity(buffer, buffer->position + otherSize)) {
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

CFL_BOOL cfl_buffer_putFormatArgs(CFL_BUFFERP buffer, const char *format, va_list varArgs) {
   int iLen;
   va_list args_copy;

   va_copy(args_copy, varArgs);
   iLen = vsnprintf(NULL, 0, format, args_copy);
   va_end(args_copy);

   if (iLen > 0) {
      CFL_UINT32 strLen = (CFL_UINT32)iLen;
      char previousNullTerminator;
      if (!buffer_put_val(buffer, &strLen, sizeof(strLen))) {
         return CFL_FALSE;
      }
      // +1 for guaranteed buffer->position + strLen is a valid buffer position
      if (!ensureCapacity(buffer, buffer->position + strLen + 1)) {
         return CFL_FALSE;
      }
      previousNullTerminator = buffer->data[buffer->position + strLen];
      iLen = vsnprintf((char *)&buffer->data[buffer->position], iLen + 1, format, varArgs);
      buffer->position += strLen;
      if (buffer->position > buffer->length) {
         buffer->length = buffer->position;
      }
      buffer->data[buffer->position] = previousNullTerminator;
      return CFL_TRUE;
   } else if (iLen == 0) {
      return cfl_buffer_putUInt32(buffer, 0);
   }
   return CFL_FALSE;
}

CFL_BOOL cfl_buffer_putFormat(CFL_BUFFERP buffer, const char *format, ...) {
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
   toBuffer->data = fromBuffer->data;
   toBuffer->length = fromBuffer->length;
   toBuffer->position = fromBuffer->position;
   toBuffer->capacity = fromBuffer->capacity;
   toBuffer->endian = fromBuffer->endian;
   fromBuffer->data = NULL;
   fromBuffer->length = 0;
   fromBuffer->position = 0;
   fromBuffer->capacity = 0;
   return CFL_TRUE;
}
