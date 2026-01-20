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

#ifndef CFL_LOCK_H_

#define CFL_LOCK_H_

#include "cfl_array.h"
#include "cfl_thread.h"
#include "cfl_types.h"


#ifdef __cplusplus
extern "C" {
#endif

#if !defined(__BORLANDC__)
#define _CFL_CONDITION_VAR
#endif

#define CFL_LOCK_SUCCESS 0
#define CFL_LOCK_ERROR 1
#define CFL_LOCK_TIMEOUT 2

#if defined(CFL_OS_WINDOWS)
#include "windows.h"

#ifdef SRWLOCK_INIT
typedef SRWLOCK CFL_LOCK_HANDLE;
#else
typedef CRITICAL_SECTION CFL_LOCK_HANDLE;
#endif
#if defined(CONDITION_VARIABLE_INIT)
typedef CONDITION_VARIABLE CFL_CONDITION_HANDLE;
#else
typedef int CFL_CONDITION_HANDLE;
#endif
#else
#include <errno.h>
#include <pthread.h>

typedef pthread_mutex_t CFL_LOCK_HANDLE;
typedef pthread_cond_t CFL_CONDITION_HANDLE;
#endif

typedef struct _CFL_CONDITION_VARIABLE {
  CFL_CONDITION_HANDLE handle;
  CFL_BOOL isAllocated;
} CFL_CONDITION_VARIABLE, *CFL_CONDITION_VARIABLEP;

typedef struct _CFL_LOCK {
  CFL_LOCK_HANDLE handle;
  CFL_BOOL isAllocated;
} CFL_LOCK, *CFL_LOCKP;

/**
 * @brief Creates a new lock.
 * @return Pointer to the new lock, or NULL if allocation fails.
 */
extern CFL_LOCKP cfl_lock_new(void);

/**
 * @brief Frees the memory used by a lock.
 * @param pLock Pointer to the lock to free.
 */
extern void cfl_lock_free(CFL_LOCKP pLock);

/**
 * @brief Initializes an existing lock structure.
 * @param pLock Pointer to the lock structure to initialize.
 */
extern void cfl_lock_init(CFL_LOCKP pLock);

/**
 * @brief Attempts to acquire a lock without blocking.
 * @param pLock Pointer to the lock to acquire.
 * @return CFL_TRUE if lock was acquired, CFL_FALSE if lock is held by another
 * thread.
 */
extern CFL_BOOL cfl_lock_tryAcquire(CFL_LOCKP pLock);

/**
 * @brief Acquires a lock (enters critical section).
 * @param pLock Pointer to the lock to acquire.
 * @note This function blocks until the lock is acquired.
 */
extern void cfl_lock_acquire(CFL_LOCKP pLock);

/**
 * @brief Releases a lock (leaves critical section).
 * @param pLock Pointer to the lock to release.
 */
extern void cfl_lock_release(CFL_LOCKP pLock);

/**
 * @brief Initializes an existing condition variable structure.
 * @param pVar Pointer to the condition variable to initialize.
 */
extern void cfl_lock_initConditionVar(CFL_CONDITION_VARIABLEP pVar);

/**
 * @brief Creates a new condition variable.
 * @return Pointer to the new condition variable, or NULL if allocation fails.
 */
extern CFL_CONDITION_VARIABLEP cfl_lock_newConditionVar(void);

/**
 * @brief Frees the memory used by a condition variable.
 * @param pVar Pointer to the condition variable to free.
 */
extern void cfl_lock_freeConditionVar(CFL_CONDITION_VARIABLEP pVar);

/**
 * @brief Waits indefinitely on a condition variable.
 * @param pLock Pointer to the associated lock (must be held).
 * @param pVar Pointer to the condition variable to wait on.
 * @return CFL_TRUE if successful, CFL_FALSE on error.
 */
extern CFL_BOOL cfl_lock_conditionWait(CFL_LOCKP pLock,
                                       CFL_CONDITION_VARIABLEP pVar);

/**
 * @brief Waits on a condition variable with a timeout.
 * @param pLock Pointer to the associated lock (must be held).
 * @param pVar Pointer to the condition variable to wait on.
 * @param timeout Timeout in milliseconds.
 * @return CFL_LOCK_SUCCESS, CFL_LOCK_TIMEOUT, or CFL_LOCK_ERROR.
 */
extern CFL_UINT8 cfl_lock_conditionWaitTimeout(CFL_LOCKP pLock,
                                               CFL_CONDITION_VARIABLEP pVar,
                                               CFL_UINT32 timeout);

/**
 * @brief Wakes a single thread waiting on a condition variable.
 * @param pVar Pointer to the condition variable.
 */
extern void cfl_lock_conditionWake(CFL_CONDITION_VARIABLEP pVar);

/**
 * @brief Wakes all threads waiting on a condition variable.
 * @param pVar Pointer to the condition variable.
 */
extern void cfl_lock_conditionWakeAll(CFL_CONDITION_VARIABLEP pVar);

/**
 * @brief Returns the last error code from the operating system.
 * @return Error code (errno on Linux, GetLastError() on Windows).
 */
extern CFL_INT32 cfl_lock_lastErrorCode(void);

/* ============================================================================
 * Reentrant Lock Functions
 * A reentrant lock allows the same thread to acquire the lock multiple times.
 * ============================================================================
 */

/**
 * @brief Reentrant (recursive) lock structure.
 *
 * This lock can be acquired multiple times by the same thread without
 * causing a deadlock. The lock must be released the same number of times
 * it was acquired.
 */
typedef struct _CFL_RLOCK {
  CFL_LOCK_HANDLE handle; /**< Platform-specific lock handle */
  CFL_THREAD_ID owner;    /**< Thread ID of the lock owner */
  CFL_UINT32 count;       /**< Number of times the lock has been acquired */
  CFL_BOOL isAllocated; /**< Whether the structure was dynamically allocated */
} CFL_RLOCK, *CFL_RLOCKP;

/**
 * @brief Creates a new reentrant lock.
 * @return Pointer to the new reentrant lock, or NULL if allocation fails.
 */
extern CFL_RLOCKP cfl_rlock_new(void);

/**
 * @brief Initializes an existing reentrant lock structure.
 * @param pLock Pointer to the reentrant lock structure to initialize.
 */
extern void cfl_rlock_init(CFL_RLOCKP pLock);

/**
 * @brief Frees the memory used by a reentrant lock.
 * @param pLock Pointer to the reentrant lock to free.
 */
extern void cfl_rlock_free(CFL_RLOCKP pLock);

/**
 * @brief Acquires a reentrant lock.
 * @param pLock Pointer to the reentrant lock to acquire.
 * @note If the current thread already holds the lock, this increments
 *       the recursion count and returns immediately.
 * @note This function blocks until the lock is acquired.
 */
extern void cfl_rlock_acquire(CFL_RLOCKP pLock);

/**
 * @brief Attempts to acquire a reentrant lock without blocking.
 * @param pLock Pointer to the reentrant lock to acquire.
 * @return CFL_TRUE if lock was acquired, CFL_FALSE if lock is held by another
 * thread.
 */
extern CFL_BOOL cfl_rlock_tryAcquire(CFL_RLOCKP pLock);

/**
 * @brief Releases a reentrant lock.
 * @param pLock Pointer to the reentrant lock to release.
 * @note The lock is only fully released when the recursion count reaches zero.
 */
extern void cfl_rlock_release(CFL_RLOCKP pLock);

/**
 * @brief Gets the current recursion count of the lock.
 * @param pLock Pointer to the reentrant lock.
 * @return The number of times the lock has been acquired by the owning thread.
 */
extern CFL_UINT32 cfl_rlock_getCount(CFL_RLOCKP pLock);

/**
 * @brief Checks if the lock is held by the current thread.
 * @param pLock Pointer to the reentrant lock.
 * @return CFL_TRUE if the current thread holds the lock, CFL_FALSE otherwise.
 */
extern CFL_BOOL cfl_rlock_isHeldByCurrentThread(CFL_RLOCKP pLock);

#ifdef __cplusplus
}
#endif

#endif
