/**
 * @file cfl_iterator.h
 * @brief Generic iterator interface.
 *
 * This module defines a generic iterator interface used for traversing
 * various collection types in the library (arrays, lists, maps, etc.).
 */

#ifndef _CFL_ITERATOR_H_

#define _CFL_ITERATOR_H_

#if defined(__cplusplus)
extern "C" {
#endif

#if defined(__BORLANDC__) && __BORLANDC__ < 0x0600
#include <cstddef.h>
#endif
#include "cfl_types.h"

struct _CFL_ITERATOR;
typedef struct _CFL_ITERATOR CFL_ITERATOR;
typedef CFL_ITERATOR *CFL_ITERATORP;

struct _CFL_ITERATOR_CLASS;
typedef struct _CFL_ITERATOR_CLASS CFL_ITERATOR_CLASS;
typedef CFL_ITERATOR_CLASS *CFL_ITERATOR_CLASSP;

typedef void (*CFL_ITERATOR_FUNC)(void *);

/**
 * @brief Virtual function table for iterators.
 */
struct _CFL_ITERATOR_CLASS {
  CFL_BOOL (*has_next)(CFL_ITERATORP it); /**< Check if next element exists */
  void *(*next)(CFL_ITERATORP it);        /**< Move to next and return value */
  void *(*current_value)(CFL_ITERATORP it); /**< Get current value */
  void (*remove)(CFL_ITERATORP it);         /**< Remove current element */
  void (*free)(CFL_ITERATORP it);           /**< Free the iterator */
  void (*first)(CFL_ITERATORP it);          /**< Move to first element */
  CFL_BOOL (*has_previous)
  (CFL_ITERATORP it);                  /**< Check if previous element exists */
  void *(*previous)(CFL_ITERATORP it); /**< Move to previous and return value */
  void (*last)(CFL_ITERATORP it);      /**< Move to last element */
  void (*add)(CFL_ITERATORP it,
              void *value); /**< Add element at current position */
};

/**
 * @brief Generic iterator structure.
 */
struct _CFL_ITERATOR {
  CFL_ITERATOR_CLASS *itClass; /**< Pointer to implementation functions */
};

/**
 * @brief Creates a new iterator (base constructor).
 * @param dataSize Size of the iterator implementation structure.
 * @return Pointer to the new iterator.
 */
extern CFL_ITERATORP cfl_iterator_new(size_t dataSize);

/**
 * @brief Gets access to the iterator's implementation-specific data.
 * @param it The iterator.
 * @return Pointer to the data.
 */
extern void *cfl_iterator_data(CFL_ITERATORP it);

/**
 * @brief Frees the iterator.
 * @param it The iterator to free.
 */
extern void cfl_iterator_free(CFL_ITERATORP it);

/**
 * @brief Checks if there is a previous element.
 * @param it The iterator.
 * @return CFL_TRUE if previous exists, CFL_FALSE otherwise.
 */
extern CFL_BOOL cfl_iterator_hasPrevious(CFL_ITERATORP it);

/**
 * @brief Checks if there is a next element.
 * @param it The iterator.
 * @return CFL_TRUE if next exists, CFL_FALSE otherwise.
 */
extern CFL_BOOL cfl_iterator_hasNext(CFL_ITERATORP it);

/**
 * @brief Advances the iterator and returns the next element.
 * @param it The iterator.
 * @return Pointer to the next element, or NULL if end reached.
 */
extern void *cfl_iterator_next(CFL_ITERATORP it);

/**
 * @brief Returns the current element without moving the iterator.
 * @param it The iterator.
 * @return Pointer to the current element.
 */
extern void *cfl_iterator_value(CFL_ITERATORP it);

/**
 * @brief Removes the current element from the underlying collection.
 * @param it The iterator.
 */
extern void cfl_iterator_remove(CFL_ITERATORP it);

/**
 * @brief Resets the iterator to the beginning of the collection.
 * @param it The iterator.
 */
extern void cfl_iterator_first(CFL_ITERATORP it);

/**
 * @brief Moves the iterator back and returns the previous element.
 * @param it The iterator.
 * @return Pointer to the previous element, or NULL if start reached.
 */
extern void *cfl_iterator_previous(CFL_ITERATORP it);

/**
 * @brief Moves the iterator to the end of the collection.
 * @param it The iterator.
 */
extern void cfl_iterator_last(CFL_ITERATORP it);

/**
 * @brief Adds an element to the collection at the current position.
 * @param it The iterator.
 * @param value Pointer to the value to add.
 */
extern void cfl_iterator_add(CFL_ITERATORP it, void *value);

#if defined(__cplusplus)
}
#endif

#endif