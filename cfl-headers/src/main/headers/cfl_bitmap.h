/**
 * @file cfl_bitmap.h
 * @brief Bitmap (bit array) data structure implementation.
 *
 * This module provides a bitmap data structure for efficiently storing
 * and manipulating individual bits. Useful for flags, sets, and
 * memory-efficient boolean arrays.
 */

#ifndef CFL_BITMAP_H_

#define CFL_BITMAP_H_

#include "cfl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Bitmap structure for storing individual bits.
 */
typedef struct _CFL_BITMAP {
  CFL_UINT8 *map;    /**< Pointer to the byte array storing the bits */
  CFL_UINT16 uiSize; /**< Number of bits in the bitmap */
} CFL_BITMAP, *CFL_BITMAPP;

/**
 * @brief Creates a new bitmap with the specified number of bits.
 * @param numBits Number of bits to allocate in the bitmap.
 * @return Pointer to the new bitmap, or NULL if allocation fails.
 * @note All bits are initially set to 0.
 */
extern CFL_BITMAPP cfl_bitmap_new(CFL_UINT16 numBits);

/**
 * @brief Frees the memory used by a bitmap.
 * @param pBitMap Pointer to the bitmap to free.
 */
extern void cfl_bitmap_free(CFL_BITMAPP pBitMap);

/**
 * @brief Sets a bit at the specified position to 1.
 * @param pBitMap Pointer to the bitmap.
 * @param uiPos Position of the bit to set (0-indexed).
 */
extern void cfl_bitmap_set(CFL_BITMAPP pBitMap, CFL_UINT16 uiPos);

/**
 * @brief Resets a bit at the specified position to 0.
 * @param pBitMap Pointer to the bitmap.
 * @param uiPos Position of the bit to reset (0-indexed).
 */
extern void cfl_bitmap_reset(CFL_BITMAPP pBitMap, CFL_UINT16 uiPos);

/**
 * @brief Gets the value of a bit at the specified position.
 * @param bitMap Pointer to the bitmap.
 * @param uiPos Position of the bit to get (0-indexed).
 * @return 1 if the bit is set, 0 if the bit is clear.
 */
extern CFL_UINT8 cfl_bitmap_get(const CFL_BITMAPP bitMap, CFL_UINT16 uiPos);

/**
 * @brief Clears all bits in the bitmap (sets all to 0).
 * @param pBitMap Pointer to the bitmap.
 */
extern void cfl_bitmap_clear(CFL_BITMAPP pBitMap);

/**
 * @brief Checks if two bitmaps are equal.
 * @param pBitMap1 Pointer to the first bitmap.
 * @param pBitMap2 Pointer to the second bitmap.
 * @return CFL_TRUE if both bitmaps have the same size and all bits match,
 *         CFL_FALSE otherwise.
 */
extern CFL_BOOL cfl_bitmap_equals(CFL_BITMAPP pBitMap1, CFL_BITMAPP pBitMap2);

/**
 * @brief Checks if one bitmap is a subset of another.
 * @param pBitMap1 Pointer to the potential subset bitmap.
 * @param pBitMap2 Pointer to the potential superset bitmap.
 * @return CFL_TRUE if all set bits in pBitMap1 are also set in pBitMap2,
 *         CFL_FALSE otherwise.
 */
extern CFL_BOOL cfl_bitmap_isSubSet(CFL_BITMAPP pBitMap1, CFL_BITMAPP pBitMap2);

/**
 * @brief Creates a copy of a bitmap.
 * @param pBitMap Pointer to the bitmap to clone.
 * @return Pointer to the new cloned bitmap, or NULL if allocation fails.
 */
extern CFL_BITMAPP cfl_bitmap_clone(const CFL_BITMAPP pBitMap);

#ifdef __cplusplus
}
#endif

#endif
