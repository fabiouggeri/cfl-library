/**
 * @file cfl_number.h
 * @brief Arbitrary precision number handling.
 *
 * This module provides structures and functions for handling numbers with
 * arbitrary precision and scale (Big Decimal/Big Integer style).
 */

#ifndef CFL_NUMBER_H_

#define CFL_NUMBER_H_

#include "cfl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// #define CLF_NUM(name, size) uint32_t name[sizeof(int)+sizeof(size_t)+sizeof(size_t)+sizeof(size_t)+size]

/**
 * @brief Structure representing the magnitude of the number (bits).
 */
typedef struct _CFL_NUM_BITS {
    CFL_UINT16  numBits;      /**< Number of valid bits used */
    CFL_UINT16  wordCapacity; /**< Capacity of the words array */
    CFL_UINT32 *words;        /**< Array of 32-bit words (LSB-first) */
} CFL_NUM_BITS, *CFL_NUM_BITSP;


/**
 * @brief Arbitrary precision number structure.
 */
typedef struct _CFL_NUMBER {
    int          sign;       /**< Sign: -1, 0, +1 */
    CFL_UINT16   scale;      /**< Decimal digits after the decimal point */
    CFL_NUM_BITS magnitude;  /**< Non-negative magnitude value */
} CFL_NUMBER, *CFL_NUMBERP;

/**
 * @brief Initializes a number structure.
 * @param x Pointer to the number.
 */
extern void cfl_number_init(CFL_NUMBER *x);

/**
 * @brief Frees a number structure.
 * @param x Pointer to the number.
 */
extern void cfl_number_free(CFL_NUMBER *x);

/**
 * @brief Converts number to a string.
 * @param x Pointer to the number.
 * @return C-string representation (must be freed by caller).
 */
extern char *cfl_number_to_string(const CFL_NUMBER *x);

/**
 * @brief Creates a number from a string.
 * @param s String representation of number.
 * @return The parsed number valid by value.
 */
extern CFL_NUMBER cfl_number_from_string(const char *s);

/**
 * @brief Creates a number from a 64-bit integer.
 * @param v Integer value.
 * @return The number by value.
 */
extern CFL_NUMBER cfl_number_from_int64(CFL_INT64 v);

/**
 * @brief Divides two numbers.
 * @param A Dividend.
 * @param B Divisor.
 * @param out_scale Desired scale for the result.
 * @param Out Pointer to store result.
 * @return non-zero on success.
 */
extern int cfl_number_div(const CFL_NUMBER *A, const CFL_NUMBER *B, CFL_UINT16 out_scale, CFL_NUMBER *Out);

/**
 * @brief Multiplies two numbers.
 * @param A First operand.
 * @param B Second operand.
 * @return Result A * B.
 */
extern CFL_NUMBER cfl_number_mul(const CFL_NUMBER *A, const CFL_NUMBER *B);

/**
 * @brief Subtracts two numbers.
 * @param A First operand.
 * @param B Second operand.
 * @return Result A - B.
 */
extern CFL_NUMBER cfl_number_sub(const CFL_NUMBER *A, const CFL_NUMBER *B);

/**
 * @brief Adds two numbers.
 * @param A First operand.
 * @param B Second operand.
 * @return Result A + B.
 */
extern CFL_NUMBER cfl_number_add(const CFL_NUMBER *A, const CFL_NUMBER *B);

/**
 * @brief Creates a number from a double.
 * @param val Double value.
 * @param scale Precision scale.
 * @return The number by value.
 */
extern CFL_NUMBER cfl_number_from_double(double val, CFL_UINT16 scale);

/**
 * @brief Converts number to double.
 * @param n Pointer to number.
 * @return double approximation.
 */
extern double cfl_number_to_double(const CFL_NUMBER *n);

#ifdef __cplusplus
}
#endif

#endif
