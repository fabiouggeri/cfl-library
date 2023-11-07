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

#ifndef _CFL_TYPES_H_

#define _CFL_TYPES_H_

#if defined(__BORLANDC__)
   #include <winsock2.h>
   #include <windows.h>
   #include <io.h>
#elif defined(_MSC_VER)
   #include <winsock2.h>
   #include <um\ws2ipdef.h>
   #include <um\ws2tcpip.h>
   #include <windows.h>
#elif defined(__MINGW64__) || defined(__MINGW32__)
   #include <winsock2.h>
   #include <ws2ipdef.h>
   #include <ws2tcpip.h>
   #include <windows.h>
#elif defined(__linux__)
   #include <sys/socket.h>
   #include <unistd.h>
   #include <netdb.h>
   #include <pthread.h>
   #include <errno.h>
   #include <sched.h>
#endif

#include "cfl_ints.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef int8_t        CFL_INT8;
typedef uint8_t       CFL_UINT8;
typedef int16_t       CFL_INT16;
typedef uint16_t      CFL_UINT16;
typedef int32_t       CFL_INT32;
typedef uint32_t      CFL_UINT32;
typedef int64_t       CFL_INT64;
typedef uint64_t      CFL_UINT64;
typedef double        CFL_DOUBLE;
typedef float         CFL_FLOAT;
typedef double        CFL_FLOAT64;
typedef float         CFL_FLOAT32;
typedef unsigned char CFL_BOOL;

#if defined(_WIN32)
   #define CFL_OS_WINDOWS
   #define CFL_SOCKET               SOCKET
   #define CFL_INVALID_SOCKET       INVALID_SOCKET
   #define CFL_HANDLE               HANDLE
   #define CFL_INVALID_HANDLE_VALUE INVALID_HANDLE_VALUE
   #define CFL_SOCKET_ERROR         SOCKET_ERROR
   #define CFL_WAIT_FOREVER         INFINITE
   typedef HANDLE                   CFL_THREAD_HANDLE;
   typedef DWORD                    CFL_THREAD_ID;
   #ifdef SRWLOCK_INIT  
      typedef SRWLOCK               CFL_LOCK_HANDLE;
   #else  
      typedef CRITICAL_SECTION      CFL_LOCK_HANDLE;
   #endif
   #if defined(CONDITION_VARIABLE_INIT)
      typedef CONDITION_VARIABLE    CFL_CONDITION_HANDLE;
   #else
      typedef int                   CFL_CONDITION_HANDLE;
   #endif
   
   #if defined(_WIN64)
      #define CFL_ARCH_64
   #else
      #define CFL_ARCH_32
   #endif
#elif defined(__linux__)
   #define CFL_OS_LINUX
   #define CFL_SOCKET               int
   #define CFL_INVALID_SOCKET       -1
   #define CFL_HANDLE               int
   #define CFL_INVALID_HANDLE_VALUE -1
   #define CFL_SOCKET_ERROR         -1
   #define CFL_WAIT_FOREVER         0xFFFFFFFF
   typedef pthread_t                CFL_THREAD_HANDLE;
   typedef pthread_t                CFL_THREAD_ID;
   typedef pthread_mutex_t          CFL_LOCK_HANDLE;
   typedef pthread_cond_t           CFL_CONDITION_HANDLE;
   #if defined (__GNUC__)
      #if defined(__x86_64__) || defined(__ppc64__)
         #define CFL_ARCH_64
      #else
         #define CFL_ARCH_32
      #endif
   #endif
#endif

#define CFL_FALSE 0
#define CFL_TRUE  1

#ifndef NULL
   #define NULL    0
#endif

#define CFL_MAX_INT_FLOAT  16777216

#define CFL_MAX_INT_DOUBLE 9007199254740992

#define CFL_UNUSED(x) (void)(x)

#if __STDC_VERSION__ >= 199901L
   #define CFL_INLINE inline
#else
   #define CFL_INLINE
#endif

struct _CFL_HASH_ENTRY;
typedef struct _CFL_HASH_ENTRY CFL_HASH_ENTRY;
typedef struct _CFL_HASH_ENTRY *CFL_HASH_ENTRYP;

struct _CFL_HASH;
typedef struct _CFL_HASH CFL_HASH;
typedef struct _CFL_HASH *CFL_HASHP;

struct _CFL_ARRAY;
typedef struct _CFL_ARRAY CFL_ARRAY;
typedef struct _CFL_ARRAY *CFL_ARRAYP;

struct _CFL_LIST;
typedef struct _CFL_LIST CFL_LIST;
typedef struct _CFL_LIST *CFL_LISTP;

struct _CFL_LLIST;
typedef struct _CFL_LLIST CFL_LLIST;
typedef struct _CFL_LLIST *CFL_LLISTP;

struct _CFL_STR;
typedef struct _CFL_STR CFL_STR;
typedef struct _CFL_STR *CFL_STRP;

struct _CFL_BITMAP;
typedef struct _CFL_BITMAP CFL_BITMAP;
typedef struct _CFL_BITMAP *CFL_BITMAPP;

struct _CFL_LOCK;
typedef struct _CFL_LOCK CFL_LOCK;
typedef struct _CFL_LOCK *CFL_LOCKP;

struct _CFL_CONDITION_VARIABLE;
typedef struct _CFL_CONDITION_VARIABLE CFL_CONDITION_VARIABLE;
typedef struct _CFL_CONDITION_VARIABLE *CFL_CONDITION_VARIABLEP;

struct _CFL_ITERATOR;
typedef struct _CFL_ITERATOR CFL_ITERATOR;
typedef struct _CFL_ITERATOR *CFL_ITERATORP;

typedef void (*CFL_ITERATOR_FUNC)(void *);

struct _CFL_ITERATOR_CLASS;
typedef struct _CFL_ITERATOR_CLASS CFL_ITERATOR_CLASS;
typedef struct _CFL_ITERATOR_CLASS *CFL_ITERATOR_CLASSP;

struct _CFL_BTREE_NODE;
typedef struct _CFL_BTREE_NODE CFL_BTREE_NODE;
typedef struct _CFL_BTREE_NODE *CFL_BTREE_NODEP;

typedef CFL_INT16 (* BTREE_CMP_VALUE_FUNC)(void *pValue1, void *pValue2, CFL_BOOL bExact);
typedef void (* BTREE_FREE_KEY_FUNC)(void *pValue);
typedef CFL_BOOL (* BTREE_WALK_CALLBACK)(void *pValue);

struct _CFL_BTREE;
typedef struct _CFL_BTREE CFL_BTREE;
typedef struct _CFL_BTREE *CFL_BTREEP;

struct _CFL_BUFFER;
typedef struct _CFL_BUFFER CFL_BUFFER;
typedef struct _CFL_BUFFER *CFL_BUFFERP;

struct _CFL_SYNC_QUEUE;
typedef struct _CFL_SYNC_QUEUE CFL_SYNC_QUEUE;
typedef struct _CFL_SYNC_QUEUE *CFL_SYNC_QUEUEP;

struct _CFL_DATE;
typedef struct _CFL_DATE CFL_DATE;
typedef struct _CFL_DATE *CFL_DATEP;

#define CFL_NO_ERROR_TYPE 0
#define CFL_NO_ERROR_CODE 0

struct _CFL_ERROR;
typedef struct _CFL_ERROR CFL_ERROR;
typedef struct _CFL_ERROR *CFL_ERRORP;

struct _CFL_THREAD;
typedef struct _CFL_THREAD CFL_THREAD;
typedef struct _CFL_THREAD *CFL_THREADP;

struct _CFL_THREAD_VARIABLE;
typedef struct _CFL_THREAD_VARIABLE CFL_THREAD_VARIABLE;
typedef struct _CFL_THREAD_VARIABLE *CFL_THREAD_VARIABLEP;

struct _CFL_EVENT;
typedef struct _CFL_EVENT CFL_EVENT;
typedef struct _CFL_EVENT *CFL_EVENTP;

struct _CFL_SQL;
typedef struct _CFL_SQL CFL_SQL;
typedef struct _CFL_SQL *CFL_SQLP;

struct _CFL_SQL_QUERY;
typedef struct _CFL_SQL_QUERY CFL_SQL_QUERY;
typedef struct _CFL_SQL_QUERY *CFL_SQL_QUERYP;

struct _CFL_SQL_INSERT;
typedef struct _CFL_SQL_INSERT CFL_SQL_INSERT;
typedef struct _CFL_SQL_INSERT *CFL_SQL_INSERTP;

struct _CFL_SQL_UPDATE;
typedef struct _CFL_SQL_UPDATE CFL_SQL_UPDATE;
typedef struct _CFL_SQL_UPDATE *CFL_SQL_UPDATEP;

struct _CFL_SQL_DELETE;
typedef struct _CFL_SQL_DELETE CFL_SQL_DELETE;
typedef struct _CFL_SQL_DELETE *CFL_SQL_DELETEP;

struct _CFL_SQL_CUSTOM;
typedef struct _CFL_SQL_CUSTOM CFL_SQL_CUSTOM;
typedef struct _CFL_SQL_CUSTOM *CFL_SQL_CUSTOMP;

struct _CFL_SQL_FUN;
typedef struct _CFL_SQL_FUN CFL_SQL_FUN;
typedef struct _CFL_SQL_FUN *CFL_SQL_FUNP;

struct _CFL_SQL_DOUBLE_OP;
typedef struct _CFL_SQL_DOUBLE_OP CFL_SQL_DOUBLE_OP;
typedef struct _CFL_SQL_DOUBLE_OP *CFL_SQL_DOUBLE_OPP;

struct _CFL_SQL_SINGLE_OP;
typedef struct _CFL_SQL_SINGLE_OP CFL_SQL_SINGLE_OP;
typedef struct _CFL_SQL_SINGLE_OP *CFL_SQL_SINGLE_OPP;

struct _CFL_SQL_WRAP;
typedef struct _CFL_SQL_WRAP CFL_SQL_WRAP;
typedef struct _CFL_SQL_WRAP *CFL_SQL_WRAPP;

struct _CFL_SQL_BLOCK;
typedef struct _CFL_SQL_BLOCK CFL_SQL_BLOCK;
typedef struct _CFL_SQL_BLOCK *CFL_SQL_BLOCKP;

struct _CFL_SQL_BUILDER;
typedef struct _CFL_SQL_BUILDER CFL_SQL_BUILDER;
typedef struct _CFL_SQL_BUILDER *CFL_SQL_BUILDERP;

struct _CFL_MAP;
typedef struct _CFL_MAP CFL_MAP;
typedef struct _CFL_MAP *CFL_MAPP;

struct _CFL_LOGGER;
typedef struct _CFL_LOGGER *CFL_LOGGERP;

struct _CFL_MAPSTR;
typedef struct _CFL_MAPSTR CFL_MAPSTR;
typedef struct _CFL_MAPSTR *CFL_MAPSTRP;

struct _CFL_MAPSTR_ENTRY;
typedef struct _CFL_MAPSTR_ENTRY CFL_MAPSTR_ENTRY;
typedef struct _CFL_MAPSTR_ENTRY *CFL_MAPSTR_ENTRYP;

#ifdef __cplusplus
}
#endif

#endif
