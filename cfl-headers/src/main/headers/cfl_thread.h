/**
 * @file cfl_thread.h
 * @brief Thread management and thread-local storage implementation.
 *
 * This module provides a cross-platform abstraction for thread creation,
 * management, synchronization, and thread-local storage. It supports both
 * Windows and POSIX (pthread) threading models.
 */

#ifndef CFL_THREAD_H_

#define CFL_THREAD_H_

#include "cfl_str.h"
#include "cfl_types.h"

#if defined(CFL_OS_WINDOWS)
#include <windows.h>

typedef HANDLE CFL_THREAD_HANDLE;
typedef DWORD CFL_THREAD_ID;

#if defined(_MSC_VER) && (_MSC_VER <= 1500)
#define CFL_THREAD_WINRAWAPI
#endif

#if defined(__BORLANDC__) && __BORLANDC__ < 0x0600
#define YieldProcessor() Sleep(0)
#define CFL_THREAD_WINRAWAPI
#endif

#else

#include <errno.h>
#include <pthread.h>
#include <sched.h>
#include <signal.h>

typedef pthread_t CFL_THREAD_HANDLE;
typedef pthread_t CFL_THREAD_ID;

#endif
#ifdef __cplusplus
extern "C" {
#endif

/** @brief Thread status: thread has been created but not started */
#define CFL_THREAD_CREATED 0x00
/** @brief Thread status: thread is currently running */
#define CFL_THREAD_RUNNING 0x01
/** @brief Thread status: thread has finished execution normally */
#define CFL_THREAD_FINISHED 0x02
/** @brief Thread status: thread was killed/terminated */
#define CFL_THREAD_KILLED 0x03
/** @brief Thread status: thread encountered an error */
#define CFL_THREAD_ERROR 0xFF

/**
 * @brief Macro to declare getter and setter functions for thread-local
 * variables.
 */
#define DECLARE_GET_SET(datatype, typename)                                    \
  extern datatype cfl_thread_varGet##typename(CFL_THREAD_VARIABLEP threadVar); \
  extern CFL_BOOL cfl_thread_varSet##                                          \
      typename(CFL_THREAD_VARIABLEP threadVar, datatype data);

#if defined(CFL_OS_WINDOWS)
/** @brief Initializes a thread-local variable with custom init/free functions
 */
#define CFL_THREAD_VAR_INIT(datatype, varname, initFunc, freeFunc)             \
  CFL_THREAD_VARIABLE varname = {0, TLS_OUT_OF_INDEXES, sizeof(datatype),      \
                                 initFunc, freeFunc}
/** @brief Initializes a simple thread-local variable */
#define CFL_THREAD_VAR(datatype, varname)                                      \
  CFL_THREAD_VARIABLE varname = {0, TLS_OUT_OF_INDEXES, sizeof(datatype),      \
                                 NULL, NULL}
#else
/** @brief Initializes a thread-local variable with custom init/free functions
 */
#define CFL_THREAD_VAR_INIT(datatype, varname, initFunc, freeFunc)             \
  CFL_THREAD_VARIABLE varname = {0, 0, sizeof(datatype), initFunc, freeFunc}
/** @brief Initializes a simple thread-local variable */
#define CFL_THREAD_VAR(datatype, varname)                                      \
  CFL_THREAD_VARIABLE varname = {0, 0, sizeof(datatype), NULL, NULL}
#endif

/** @brief Function pointer type for thread-local variable init/free callbacks
 */
typedef void (*CFL_THREAD_VAR_FUNC)(void *param);

/** @brief Function pointer type for thread entry point */
typedef void (*CFL_THREAD_FUNC)(void *param);

/**
 * @brief Union for storing different data types in thread-local storage.
 */
typedef union {
  CFL_BOOL asBoolean;    /**< Boolean value */
  CFL_INT8 asInt8;       /**< Signed 8-bit integer */
  CFL_INT16 asInt16;     /**< Signed 16-bit integer */
  CFL_INT32 asInt32;     /**< Signed 32-bit integer */
  CFL_INT64 asInt64;     /**< Signed 64-bit integer */
  CFL_UINT8 asUInt8;     /**< Unsigned 8-bit integer */
  CFL_UINT16 asUInt16;   /**< Unsigned 16-bit integer */
  CFL_UINT32 asUInt32;   /**< Unsigned 32-bit integer */
  CFL_UINT64 asUInt64;   /**< Unsigned 64-bit integer */
  CFL_FLOAT32 asFloat32; /**< 32-bit floating point */
  CFL_FLOAT64 asFloat64; /**< 64-bit floating point */
  void *asPointer;       /**< Generic pointer */
} VAR_DATA;

/**
 * @brief Thread-local variable descriptor structure.
 */
typedef struct _CFL_THREAD_VARIABLE {
#if defined(CFL_OS_WINDOWS)
  volatile LONG initialized; /**< Initialization state flag */
  DWORD storageKey;          /**< Thread-local storage key */
#else
  volatile int initialized; /**< Initialization state flag */
  pthread_key_t storageKey; /**< Thread-local storage key */
#endif
  size_t dataSize;              /**< Size of the stored data */
  CFL_THREAD_VAR_FUNC initData; /**< Optional initialization callback */
  CFL_THREAD_VAR_FUNC freeData; /**< Optional cleanup callback */
} CFL_THREAD_VARIABLE, *CFL_THREAD_VARIABLEP;

/**
 * @brief Thread-local variable data wrapper structure.
 */
typedef struct _CFL_THREAD_VAR_DATA {
  CFL_THREAD_VARIABLEP var; /**< Pointer to the variable descriptor */
  CFL_UINT8 data[1];        /**< Flexible array for actual data */
} CFL_THREAD_VAR_DATA, *CFL_THREAD_VAR_DATAP;

/**
 * @brief Thread structure containing thread state and metadata.
 */
typedef struct _CFL_THREAD {
  CFL_THREAD_FUNC func;      /**< Thread entry point function */
  void *param;               /**< Parameter passed to thread function */
  CFL_STR description;       /**< Optional thread description */
  CFL_THREAD_HANDLE handle;  /**< Platform-specific thread handle */
  CFL_BOOL manualAllocation; /**< Whether structure was manually allocated */
  CFL_BOOL joined;           /**< Whether thread has been joined */
  CFL_UINT8 status;          /**< Current thread status */
} CFL_THREAD, *CFL_THREADP;

/**
 * @brief Creates a new thread object.
 * @param func The thread entry point function.
 * @return Pointer to the new thread object, or NULL if allocation fails.
 * @note The thread is created but not started. Use cfl_thread_start() to run
 * it.
 */
extern CFL_THREADP cfl_thread_new(CFL_THREAD_FUNC func);

/**
 * @brief Creates a new thread object with a description.
 * @param func The thread entry point function.
 * @param description Human-readable description for the thread.
 * @return Pointer to the new thread object, or NULL if allocation fails.
 */
extern CFL_THREADP cfl_thread_newWithDescription(CFL_THREAD_FUNC func,
                                                 const char *description);

/**
 * @brief Frees a thread object and its resources.
 * @param thread Pointer to the thread object to free.
 * @note The thread must have completed (joined or killed) before freeing.
 */
extern void cfl_thread_free(CFL_THREADP thread);

/**
 * @brief Sets the description for a thread.
 * @param thread Pointer to the thread object.
 * @param description Human-readable description for the thread.
 */
extern void cfl_thread_setDescription(CFL_THREADP thread,
                                      const char *description);

/**
 * @brief Gets the current thread's ID.
 * @return The ID of the calling thread.
 */
extern CFL_THREAD_ID cfl_thread_id(void);

/**
 * @brief Compares two thread IDs for equality.
 * @param th1 First thread ID.
 * @param th2 Second thread ID.
 * @return CFL_TRUE if the IDs refer to the same thread, CFL_FALSE otherwise.
 */
extern CFL_BOOL cfl_thread_equals(CFL_THREAD_ID th1, CFL_THREAD_ID th2);

/**
 * @brief Gets the CFL_THREADP for the current thread.
 * @return Pointer to the current thread's CFL_THREAD structure, or NULL if not
 * available.
 */
extern CFL_THREADP cfl_thread_getCurrent(void);

/**
 * @brief Starts a thread's execution.
 * @param thread Pointer to the thread object to start.
 * @param param Parameter to pass to the thread function.
 * @return CFL_TRUE if the thread was started successfully, CFL_FALSE otherwise.
 */
extern CFL_BOOL cfl_thread_start(CFL_THREADP thread, void *param);

/**
 * @brief Waits for a thread to finish execution.
 * @param thread Pointer to the thread object to wait for.
 * @return CFL_TRUE if the wait was successful, CFL_FALSE otherwise.
 * @note This function blocks until the thread terminates.
 */
extern CFL_BOOL cfl_thread_wait(CFL_THREADP thread);

/**
 * @brief Waits for a thread to finish with a timeout.
 * @param thread Pointer to the thread object to wait for.
 * @param timeout Maximum time to wait in milliseconds. Use CFL_WAIT_FOREVER for
 * infinite wait.
 * @return CFL_TRUE if the thread finished within the timeout, CFL_FALSE if
 * timeout expired.
 */
extern CFL_BOOL cfl_thread_waitTimeout(CFL_THREADP thread, CFL_INT32 timeout);

/**
 * @brief Forcefully terminates a thread.
 * @param thread Pointer to the thread object to kill.
 * @return CFL_TRUE if the thread was killed successfully, CFL_FALSE otherwise.
 * @warning This function should be used with caution as it may leave resources
 * in an inconsistent state.
 */
extern CFL_BOOL cfl_thread_kill(CFL_THREADP thread);

/**
 * @brief Gets the current status of a thread.
 * @param thread Pointer to the thread object.
 * @return The thread's current status (CFL_THREAD_CREATED, CFL_THREAD_RUNNING,
 * etc.).
 */
extern CFL_UINT8 cfl_thread_status(const CFL_THREADP thread);

/**
 * @brief Signals that a thread has encountered an error.
 * @param thread Pointer to the thread object.
 */
extern void cfl_thread_signalError(CFL_THREADP thread);

/**
 * @brief Checks if the current thread is managed by the CFL thread system.
 * @return CFL_TRUE if the current thread has a CFL_THREAD structure, CFL_FALSE
 * otherwise.
 */
extern CFL_BOOL cfl_thread_currentIsHandled(void);

/**
 * @brief Suspends the current thread for a specified duration.
 * @param time Time to sleep in milliseconds.
 * @return CFL_TRUE on success, CFL_FALSE on error.
 */
extern CFL_BOOL cfl_thread_sleep(CFL_UINT32 time);

/**
 * @brief Yields execution to allow other threads to run.
 */
extern void cfl_thread_yield(void);

/**
 * @brief Gets the value of a thread-local variable as a raw pointer.
 * @param threadVar Pointer to the thread-local variable descriptor.
 * @return Pointer to the variable's data, or NULL if not initialized.
 */
extern void *cfl_thread_varGet(CFL_THREAD_VARIABLEP threadVar);

/**
 * @brief Sets the value of a thread-local variable from raw data.
 * @param threadVar Pointer to the thread-local variable descriptor.
 * @param data Pointer to the data to copy into the variable.
 * @return CFL_TRUE on success, CFL_FALSE on failure.
 */
extern CFL_BOOL cfl_thread_varSet(CFL_THREAD_VARIABLEP threadVar,
                                  const void *data);

/* Typed getter/setter declarations for thread-local variables */
DECLARE_GET_SET(void *, Pointer)
DECLARE_GET_SET(CFL_BOOL, Boolean)
DECLARE_GET_SET(CFL_INT8, Int8)
DECLARE_GET_SET(CFL_INT16, Int16)
DECLARE_GET_SET(CFL_INT32, Int32)
DECLARE_GET_SET(CFL_INT64, Int64)
DECLARE_GET_SET(CFL_UINT8, UInt8)
DECLARE_GET_SET(CFL_UINT16, UInt16)
DECLARE_GET_SET(CFL_UINT32, UInt32)
DECLARE_GET_SET(CFL_UINT64, UInt64)
DECLARE_GET_SET(CFL_FLOAT32, Float32)
DECLARE_GET_SET(CFL_FLOAT64, Float64)

#ifdef __cplusplus
}
#endif

#endif