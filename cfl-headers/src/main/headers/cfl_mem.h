/**
 * @file cfl_mem.h
 * @brief Memory allocation abstraction layer.
 *
 * This module provides an abstraction layer for memory allocation functions,
 * allowing custom allocators to be used throughout the library.
 */

#ifndef _CFL_MEM_H_

#define _CFL_MEM_H_

#include <stdlib.h>

#include "cfl_types.h"

/** @brief Function pointer type for malloc-like functions */
typedef void *(*CFL_MALLOC_FUNC)(size_t size);
/** @brief Function pointer type for realloc-like functions */
typedef void *(*CFL_REALLOC_FUNC)(void *ptr, size_t size);
/** @brief Function pointer type for free-like functions */
typedef void (*CFL_FREE_FUNC)(void *ptr);

/** @brief Macro for allocating memory */
#define CFL_MEM_ALLOC(s) cfl_malloc(s)
/** @brief Macro for allocating zeroed memory */
#define CFL_MEM_CALLOC(n, s) cfl_calloc(n, s)
/** @brief Macro for reallocating memory */
#define CFL_MEM_REALLOC(m, s) cfl_realloc(m, s)
/** @brief Macro for freeing memory */
#define CFL_MEM_FREE(m) cfl_free(m)

/**
 * @brief Sets custom memory allocation functions.
 * @param malloc_func Custom malloc function.
 * @param realloc_func Custom realloc function.
 * @param free_func Custom free function.
 * @note By default, the standard malloc, realloc, and free are used.
 */
extern void cfl_mem_set(CFL_MALLOC_FUNC malloc_func,
                        CFL_REALLOC_FUNC realloc_func, CFL_FREE_FUNC free_func);

/**
 * @brief Allocates memory of the specified size.
 * @param size Number of bytes to allocate.
 * @return Pointer to allocated memory, or NULL on failure.
 */
extern void *cfl_malloc(size_t size);

/**
 * @brief Allocates and zeroes memory for an array.
 * @param numElements Number of elements to allocate.
 * @param size Size of each element in bytes.
 * @return Pointer to allocated zeroed memory, or NULL on failure.
 */
extern void *cfl_calloc(size_t numElements, size_t size);

/**
 * @brief Reallocates memory to a new size.
 * @param ptr Pointer to previously allocated memory.
 * @param size New size in bytes.
 * @return Pointer to reallocated memory, or NULL on failure.
 */
extern void *cfl_realloc(void *ptr, size_t size);

/**
 * @brief Frees previously allocated memory.
 * @param ptr Pointer to memory to free.
 */
extern void cfl_free(void *ptr);

#endif
