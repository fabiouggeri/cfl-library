/**
 * @file cfl_atomic.h
 * @brief Atomic operations for thread-safe data access.
 *
 * This module provides atomic operations for various data types, ensuring
 * thread-safe read-modify-write operations without explicit locking.
 * Supports atomic set, compare-and-swap, and arithmetic/bitwise operations.
 */

#ifndef CFL_ATOMIC_H_

#define CFL_ATOMIC_H_

#include "cfl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined(CFL_OS_WINDOWS) && !defined(__GNUC__)
#define VOLATILE_PARAM volatile
#else
#define VOLATILE_PARAM
#endif

/**
 * @brief Declares atomic set and compare-and-swap functions for a data type.
 * @param datatype The C data type.
 * @param typename The type name suffix for function names.
 *
 * Generated functions:
 * - cfl_atomic_set##typename: Atomically sets a variable to a new value.
 * - cfl_atomic_compareAndSet##typename: Atomically sets a value if it matches
 * the expected old value.
 */
#define DECLARE_OPERATIONS_SET(datatype, typename)                             \
  extern datatype cfl_atomic_set##                                             \
      typename(VOLATILE_PARAM datatype * var, datatype value);                 \
  extern datatype cfl_atomic_compareAndSet##typename(                          \
      VOLATILE_PARAM datatype * var, datatype oldValue, datatype newValue);

/**
 * @brief Declares atomic arithmetic and bitwise operation functions for a data
 * type.
 * @param datatype The C data type.
 * @param typename The type name suffix for function names.
 *
 * Generated functions:
 * - cfl_atomic_add##typename: Atomically adds a value and returns the previous
 * value.
 * - cfl_atomic_sub##typename: Atomically subtracts a value and returns the
 * previous value.
 * - cfl_atomic_and##typename: Atomically performs bitwise AND and returns the
 * previous value.
 * - cfl_atomic_or##typename: Atomically performs bitwise OR and returns the
 * previous value.
 * - cfl_atomic_xor##typename: Atomically performs bitwise XOR and returns the
 * previous value.
 */
#define DECLARE_OPERATIONS_OP(datatype, typename)                              \
  extern datatype cfl_atomic_add##                                             \
      typename(VOLATILE_PARAM datatype * var, datatype value);                 \
  extern datatype cfl_atomic_sub##                                             \
      typename(VOLATILE_PARAM datatype * var, datatype value);                 \
  extern datatype cfl_atomic_and##                                             \
      typename(VOLATILE_PARAM datatype * var, datatype value);                 \
  extern datatype cfl_atomic_or##                                              \
      typename(VOLATILE_PARAM datatype * var, datatype value);                 \
  extern datatype cfl_atomic_xor##                                             \
      typename(VOLATILE_PARAM datatype * var, datatype value)

/* Boolean atomic operations (set and compare-and-swap only) */
DECLARE_OPERATIONS_SET(CFL_BOOL, Boolean);

/* 8-bit integer atomic operations */
DECLARE_OPERATIONS_SET(CFL_INT8, Int8);
DECLARE_OPERATIONS_OP(CFL_INT8, Int8);

/* 16-bit integer atomic operations */
DECLARE_OPERATIONS_SET(CFL_INT16, Int16);
DECLARE_OPERATIONS_OP(CFL_INT16, Int16);

/* 32-bit integer atomic operations */
DECLARE_OPERATIONS_SET(CFL_INT32, Int32);
DECLARE_OPERATIONS_OP(CFL_INT32, Int32);

/* 64-bit integer atomic operations (platform-dependent availability) */
#if defined(CFL_OS_WINDOWS)
#if defined(CFL_ARCH_64)
DECLARE_OPERATIONS_SET(CFL_INT64, Int64);
DECLARE_OPERATIONS_OP(CFL_INT64, Int64);
#endif
#else
DECLARE_OPERATIONS_SET(CFL_INT64, Int64);
DECLARE_OPERATIONS_OP(CFL_INT64, Int64);
#endif

/* Pointer atomic operations (set and compare-and-swap only) */
DECLARE_OPERATIONS_SET(void *, Pointer);

#ifdef __cplusplus
}
#endif

#endif
