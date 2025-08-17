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
#include "cfl_mem.h"

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

/**
 * Initializes a string structure with a given initial capacity.
 * 
 * This function initializes a CFL_STR structure, allocating memory for the string
 * data if the initial capacity is greater than 0. If memory allocation fails or
 * initial capacity is 0, the string will be initialized as an empty constant string.
 *
 * @param str Pointer to the CFL_STR structure to be initialized. If NULL, function returns without effect.
 * @param iniCapacity Initial capacity for the string buffer. If 0, no memory is allocated.
 *
 * @note The actual allocated size will be iniCapacity + 1 to accommodate the null terminator.
 * @note If memory allocation fails, the string will be initialized as an empty constant string.
 */
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

/**
 * Initializes a CFL string structure with default values.
 * Sets the string to an empty state with zero size, length and hash value.
 * The data pointer is set to an empty string literal.
 * 
 * @param str Pointer to the CFL string structure to initialize.
 *            If NULL, the function returns without doing anything.
 */
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

/**
 * @brief Initializes a CFL_STRP with a constant string buffer of specified length.
 * 
 * This function initializes a string structure with a provided buffer, setting it up
 * as a constant (non-allocated) string. If the buffer is NULL or length is 0,
 * the string is initialized as empty.
 *
 * @param str Pointer to the CFL_STRP structure to be initialized
 * @param buffer Constant character buffer to be used as string data
 * @param len Length of the buffer (not including null terminator)
 * 
 * @note The buffer is used directly without copying, so it must remain valid
 *       for the lifetime of the string
 * @note The function will do nothing if str is NULL
 */
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

/**
 * @brief Initializes a CFL string with a constant buffer
 * 
 * This function initializes a CFL string structure with a constant character buffer.
 * If the buffer is NULL, the string will be initialized with length 0.
 * 
 * @param str Pointer to the CFL string structure to be initialized
 * @param buffer Constant character buffer to initialize the string with
 * 
 * @note The buffer is not copied, the string will reference the original buffer
 */
void cfl_str_initConst(CFL_STRP str, const char *buffer) {
   cfl_str_initConstLen(str, buffer, buffer == NULL ? 0 : (CFL_UINT32) strlen(buffer));
}

/**
 * @brief Initializes a CFL string with a given buffer value
 *
 * This function initializes a CFL string structure with the contents of the provided buffer.
 * If the buffer is NULL or empty, the string will be initialized as empty.
 * The function allocates memory for the string data if needed.
 *
 * @param str Pointer to the CFL string structure to initialize
 * @param buffer Source buffer containing the initial string value (can be NULL)
 *
 * @note If memory allocation fails, the string will be initialized as empty
 * @note The string takes ownership of the allocated memory if successful
 */
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

/**
 * Creates a new string with the specified initial capacity.
 * 
 * This function allocates memory for a new string structure and its data buffer.
 * The string is initialized as empty but with space to store the specified number
 * of characters plus the null terminator.
 *
 * @param iniCapacity The initial capacity of the string buffer (number of chars that can be stored)
 *                    If 0, creates an empty string with minimum allocation.
 * 
 * @return A pointer to the newly created string structure (CFL_STRP),
 *         or NULL if memory allocation fails
 * 
 * @note The actual allocated size will be iniCapacity + 1 to accommodate the null terminator
 * @note The returned string is marked as allocated and variable data
 */
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

/**
 * Creates a new CFL string from a buffer with specified length.
 * 
 * This function allocates memory for a new string structure and copies
 * the content from the provided buffer up to the specified length.
 * The resulting string is null-terminated.
 *
 * @param buffer The source buffer containing the string content to copy.
 *               If NULL or empty, returns an empty string.
 * @param len The number of characters to copy from the buffer.
 *           If 0, returns an empty string.
 *
 * @return A pointer to the newly created CFL_STR structure, or NULL if
 *         memory allocation fails. The returned string must be freed
 *         using cfl_str_free() when no longer needed.
 */
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

/**
 * Creates a new CFL string from a C-style string buffer.
 *
 * @param buffer The source C-style string buffer to create the CFL string from.
 *               If NULL, an empty string will be created.
 * @return A new CFL string pointer (CFL_STRP) containing the contents of the buffer.
 *         The returned string must be freed using cfl_str_free when no longer needed.
 */
CFL_STRP cfl_str_newBuffer(const char *buffer) {
   return cfl_str_newBufferLen(buffer, buffer != NULL ? (CFL_UINT32) strlen(buffer) : 0);
}

/**
 * Creates a new string object with a constant buffer of specified length.
 * 
 * This function creates a new CFL_STR structure and initializes it with the provided
 * buffer and length. The buffer is not copied but directly referenced.
 * 
 * @param buffer Pointer to the constant character buffer to reference. If NULL or
 *              length is 0, an empty string will be created.
 * @param len Length of the buffer in characters
 * 
 * @return A pointer to the newly created CFL_STR structure, or NULL if memory
 *         allocation fails
 * 
 * @note The buffer is not copied, the string object will reference the original buffer.
 *       The caller must ensure the buffer remains valid for the lifetime of the string.
 */
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

/**
 * Creates a new string from a constant buffer.
 * 
 * @param buffer The constant buffer to create the string from. If NULL, an empty string is created.
 * @return A new string pointer (CFL_STRP) containing the buffer content.
 */
CFL_STRP cfl_str_newConst(const char *buffer) {
   return cfl_str_newConstLen(buffer, buffer != NULL ? (CFL_UINT32) strlen(buffer) : 0);
}

/**
 * Creates a new string by copying an existing one
 * 
 * This function creates a new CFL_STR structure and initializes it with the contents
 * of the provided string. If the source string contains variable data (isVarData is true),
 * it allocates new memory and copies the data. Otherwise, it shares the data pointer.
 * 
 * @param strSet Pointer to the source string structure to copy from.
 *               If NULL or empty, returns a new empty string.
 * 
 * @return Pointer to the newly created string structure (CFL_STRP),
 *         or NULL if memory allocation fails
 * 
 * @note The function handles both variable and fixed data strings differently:
 *       - For variable data: Allocates new memory and copies the content
 *       - For fixed data: Shares the data pointer with the source string
 */
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

/**
 * @brief Frees memory associated with a string object
 * 
 * This function deallocates memory used by a CFL_STRP string object.
 * If the string has variable data allocated, it frees that memory first.
 * If the string object itself was dynamically allocated, it frees the object as well.
 *
 * @param str Pointer to the string object to be freed
 * @note If str is NULL, the function returns without doing anything
 */
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

/**
 * Appends a character to the end of a string.
 * If the input string is NULL, creates a new string with default capacity.
 * 
 * @param str The string to append to, or NULL to create a new string
 * @param c The character to append
 * @return The resulting string, or NULL if memory allocation fails
 * 
 * @note If the input string needs to grow to accommodate the new character,
 *       it will be reallocated with increased capacity.
 * @note The hash value of the string is invalidated (set to 0) after appending.
 */
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

/**
 * Concatenates multiple strings to an existing CFL_STR object.
 * 
 * This function appends a variable number of string arguments to the given CFL_STR object.
 * If the input string object is NULL, a new string object is created with default capacity.
 * The function ensures there is enough capacity to store all appended strings.
 * 
 * @param str The target CFL_STR object to append to. If NULL, a new string is created.
 * @param buffer The first string to append, followed by additional string arguments.
 * @param ... Variable number of string arguments (must be char*), terminated by NULL.
 * @return The resulting CFL_STR object after appending all strings.
 * 
 * @note The function resets the hash value of the resulting string to 0.
 * @note All variable arguments must be of type char* and the list must be NULL-terminated.
 * 
 * Example usage:
 * CFL_STRP result = cfl_str_append(str, "Hello", " ", "World", NULL);
 */
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

/**
 * @brief Appends a buffer of a given length to a string.
 * 
 * If the input string is NULL and the buffer is valid, creates a new string with the buffer content.
 * If the input string is NULL and the buffer is invalid, creates a new empty string with default capacity.
 * If the input string exists and the buffer is valid, appends the buffer content to the existing string.
 * 
 * @param str The string to append to, or NULL to create a new string
 * @param buffer The buffer containing the characters to append
 * @param bufferLen The number of characters to append from the buffer
 * @return CFL_STRP The resulting string (either new or modified), or NULL if memory allocation fails
 */
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

/**
 * Appends a string to another string.
 * 
 * @param str The destination string to append to. If NULL, a new string will be created.
 * @param strAppend The source string to append. If NULL or empty, no append operation is performed.
 * 
 * @return The resulting string after append operation. If str is NULL and strAppend is NULL/empty,
 *         returns a new empty string with default capacity.
 * 
 * @note If the destination string doesn't have enough capacity, it will be automatically resized.
 *       The hash value of the destination string is reset to 0 after append.
 */
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

/**
 * Appends a formatted string to an existing string using variable arguments.
 * 
 * @param str The target string to append to. If NULL, a new string will be created.
 * @param format The format string specifying how to format the variable arguments.
 * @param varArgs A va_list containing the variable arguments to format.
 * 
 * @return Returns the resulting string pointer (CFL_STRP). If input str was NULL, 
 *         returns a newly allocated string. If an error occurs during allocation 
 *         or formatting, returns NULL or the original string unchanged.
 * 
 * @note The function first calculates required length, ensures capacity, then 
 *       performs the actual formatting. The string's hash value is reset to 0
 *       after modification.
 */
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

/**
 * @brief Appends formatted text to a string
 * 
 * This function appends text formatted according to the format string to the given CFL_STRP string.
 * The format string follows the same rules as printf.
 *
 * @param str The string to append to
 * @param format The format string following printf conventions
 * @param ... Variable arguments corresponding to format specifiers
 * 
 * @return The modified string (CFL_STRP)
 * 
 * @note The function uses variable arguments internally and properly handles their cleanup
 */
CFL_STRP cfl_str_appendFormat(CFL_STRP str, const char * format, ...) {
   va_list varArgs;

   va_start(varArgs, format);
   str = cfl_str_appendFormatArgs(str, format, varArgs);
   va_end(varArgs);
   return str;
}

/**
 * Sets the content of a string using a format string and variable arguments.
 * 
 * This function formats a string similar to sprintf but manages memory automatically.
 * If the input string pointer is NULL, a new string will be allocated.
 * If the formatting fails, an empty string will be returned.
 * 
 * @param str      Pointer to the string to be modified, or NULL to create a new string
 * @param format   Format string following printf conventions
 * @param varArgs  Variable argument list containing values to be formatted
 * 
 * @return         Pointer to the resulting string (may be newly allocated if input was NULL),
 *                 or NULL if memory allocation fails
 * 
 * @note The function calculates required space first to ensure proper memory allocation
 * @note The hashValue of the string is reset to 0 after modification
 */
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

/**
 * @brief Sets the string content using a formatted string with variable arguments.
 * 
 * @param str The string object to modify
 * @param format The format string following printf conventions
 * @param ... Variable arguments corresponding to the format specifiers
 * @return CFL_STRP The modified string object
 * 
 * @note This function internally uses cfl_str_setFormatArgs to perform the actual formatting
 */
CFL_STRP cfl_str_setFormat(CFL_STRP str, const char * format, ...) {
   va_list varArgs;

   va_start(varArgs, format);
   str = cfl_str_setFormatArgs(str, format, varArgs);
   va_end(varArgs);
   return str;
}

/**
 * Sets a character at the specified index in a string.
 * If the string is NULL, creates a new string with length index + 1.
 * If the index is beyond the current string length, extends the string to accommodate the new index.
 *
 * @param str The string to modify. If NULL, a new string will be created.
 * @param index The position where to set the character.
 * @param c The character to set at the specified position.
 * @return The modified string (or newly created string if input was NULL).
 */
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

/**
 * @brief Returns a pointer to the internal character array of a string.
 *
 * @param str The CFL_STRP (string pointer) to get the data from.
 * @return char* Pointer to the internal character array.
 * 
 * @warning The returned pointer should not be modified directly as it points to 
 *          the internal data of the string structure.
 */
char *cfl_str_getPtr(const CFL_STRP str) {
   return str->data;
}

/**
 * Returns a pointer to the character at the specified index in the string.
 * 
 * @param str   The string to get the character from
 * @param index The index of the character to get
 * 
 * @return A pointer to the character at the specified index, or NULL if the index
 *         is out of bounds
 */
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

/**
 * Returns the length of the given string.
 *
 * @param str Pointer to a CFL string structure
 * @return The length of the string as CFL_UINT32
 */
CFL_UINT32 cfl_str_length(const CFL_STRP str) {
   return str->length;
}

/**
 * Sets the length of a string to the specified value.
 * If the new length is greater than the current length, the additional space
 * is filled with spaces. If the new length is smaller, the string is truncated.
 * 
 * @param str Pointer to the string structure to modify
 * @param newLen The new desired length for the string
 * 
 * @note The function ensures the target string has enough capacity before sets.
 * @note The hash value is reset to 0 after modification.
 * A null terminator is always added at the end of the string.
 */
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

/**
 * @brief Clears the content of a CFL string
 *
 * Clears the string by either:
 * - Setting first character to null terminator if string has variable data
 * - Setting data pointer to empty string and size to 0 if string has fixed data
 * 
 * Also resets the hash value and length to 0.
 *
 * @param str Pointer to the CFL string to be cleared
 */
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

/**
 * Sets the string content of a CFL_STRP from another CFL_STRP source string.
 * 
 * @param str   The destination string to be modified
 * @param src   The source string to copy from
 * 
 * @return      Returns the modified destination string (str)
 *              If src is NULL or empty, sets str to empty string
 */
CFL_STRP cfl_str_setStr(CFL_STRP str, const CFL_STRP src) {
   if (src != NULL && src->length > 0) {
      return cfl_str_setValueLen(str, src->data, src->length);
   } else {
      return cfl_str_setConstLen(str, "", 0);
   }
}

/**
 * Sets or creates a string with the given buffer content.
 * 
 * @param str The target string to set. If NULL, a new string will be created.
 * @param buffer The source buffer containing the content to be set.
 * @param bufferLen The length of the source buffer.
 * 
 * @return The modified string if str was provided, or a new string if str was NULL.
 *         Returns NULL if memory allocation fails when creating a new string.
 * 
 * @note If buffer is NULL or bufferLen is 0, the string will be cleared.
 *       The function ensures the target string has enough capacity before copying.
 *       After setting, the string's hash value is reset to 0.
 */
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

/**
 * Sets the value of a string from a buffer.
 *
 * @param str The string to set the value of
 * @param buffer The source buffer containing the string to set. If NULL, the string will be empty
 * @return The string pointer with the new value set, or NULL if the operation fails
 */
CFL_STRP cfl_str_setValue(CFL_STRP str, const char *buffer) {
   return cfl_str_setValueLen(str, buffer, buffer != NULL ? (CFL_UINT32) strlen(buffer) : 0);
}

/**
 * Sets or creates a new string with constant length data.
 * 
 * If the input string pointer is NULL, creates a new string.
 * If the input string pointer exists, replaces its content.
 * The function takes ownership of the buffer memory - it will not copy the data.
 * 
 * @param str       The string to be modified, or NULL to create a new one
 * @param buffer    The buffer containing the string data
 * @param bufferLen The length of the buffer
 * @return         The modified or newly created string
 * 
 * @note If buffer is NULL or bufferLen is 0, the string will be set to empty
 * @note The buffer must remain valid for the lifetime of the string
 * @note Any previous variable data in the string will be freed
 */
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

/**
 * @brief Sets a string constant to an existing CFL_STRP structure
 * 
 * This function sets a string constant to an existing CFL_STRP structure by using
 * the provided buffer. It internally calls cfl_str_setConstLen with the buffer's length
 * calculated using strlen.
 * 
 * @param str Pointer to the CFL_STRP structure to be modified
 * @param buffer Constant string buffer to be set. If NULL, length will be set to 0
 * @return Pointer to the modified CFL_STRP structure
 * 
 * @see cfl_str_setConstLen
 */
CFL_STRP cfl_str_setConst(CFL_STRP str, const char *buffer) {
   return cfl_str_setConstLen(str, buffer, buffer != NULL ? (CFL_UINT32) strlen(buffer) : 0);
}

/**
 * Compares two strings lexicographically.
 * 
 * @param str1 First string to compare
 * @param str2 Second string to compare
 * @param bExact If TRUE, performs exact comparison. If FALSE, treats strings as equal if one is a prefix of the other
 * 
 * @return Returns:
 *         0  if strings are equal (or one is prefix of other when bExact is FALSE)
 *         -1 if str1 is lexicographically less than str2
 *         1  if str1 is lexicographically greater than str2
 * 
 * @note If str1 and str2 point to the same string or have the same data pointer,
 *       they are considered equal and function returns 0 immediately
 */
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

/**
 * Compares two strings ignoring case.
 * 
 * @param str1 Pointer to the first string to compare
 * @param str2 Pointer to the second string to compare
 * @param bExact If CFL_TRUE, performs exact comparison. If CFL_FALSE, treats strings as equal if one is a prefix of the other
 * 
 * @return Returns:
 *         - 0 if strings are equal or if one is prefix of other (when bExact is CFL_FALSE)
 *         - -1 if str1 is lexicographically less than str2
 *         - 1 if str1 is lexicographically greater than str2
 * 
 * @note The comparison is case-insensitive (e.g., 'A' equals 'a')
 * @note If str1 and str2 point to the same string or same data, returns 0 immediately
 */
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

/**
 * Checks if a string starts with another string.
 * 
 * @param str The string to check
 * @param strStart The string to find at the start
 * @return CFL_TRUE if str starts with strStart, CFL_FALSE otherwise
 */
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
