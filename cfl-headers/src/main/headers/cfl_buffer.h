/**
 * @file cfl_buffer.h
 * @brief Dynamic byte buffer implementation with serialization support.
 *
 * This module provides a dynamic buffer for reading and writing binary data
 * with support for multiple data types and endianness control.
 */

#ifndef _CFL_BUFFER_H_

#define _CFL_BUFFER_H_

#include "cfl_date.h"
#include "cfl_str.h"
#include "cfl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Big endian byte order */
#define CFL_BIG_ENDIAN 0x00
/** @brief Little endian byte order */
#define CFL_LITTLE_ENDIAN 0x01

/**
 * @brief Dynamic buffer structure for binary data.
 */
typedef struct _CFL_BUFFER {
  CFL_UINT8 *data;     /**< Pointer to buffer data */
  CFL_UINT32 length;   /**< Current data length in bytes */
  CFL_UINT32 position; /**< Current read/write position */
  CFL_UINT32 capacity; /**< Allocated capacity in bytes */
  CFL_BOOL allocated;  /**< Whether structure was dynamically allocated */
  CFL_UINT8 endian;    /**< Byte order for multi-byte values */
} CFL_BUFFER, *CFL_BUFFERP;

/**
 * @brief Initializes a buffer structure.
 * @param buffer Pointer to the buffer to initialize.
 */
extern void cfl_buffer_init(CFL_BUFFERP buffer);

/**
 * @brief Creates a new buffer with default capacity.
 * @return Pointer to the new buffer, or NULL if allocation fails.
 */
extern CFL_BUFFERP cfl_buffer_new(void);

/**
 * @brief Creates a new buffer with specified initial capacity.
 * @param initialCapacity Initial capacity in bytes.
 * @return Pointer to the new buffer, or NULL if allocation fails.
 */
extern CFL_BUFFERP cfl_buffer_newCapacity(CFL_UINT32 initialCapacity);

/**
 * @brief Creates a copy of a buffer.
 * @param other Pointer to the buffer to clone.
 * @return Pointer to the new cloned buffer, or NULL if allocation fails.
 */
extern CFL_BUFFERP cfl_buffer_clone(CFL_BUFFERP other);

/**
 * @brief Frees the memory used by a buffer.
 * @param buffer Pointer to the buffer to free.
 */
extern void cfl_buffer_free(CFL_BUFFERP buffer);

/**
 * @brief Gets a pointer to the buffer's data.
 * @param buffer Pointer to the buffer.
 * @return Pointer to the data array.
 */
extern CFL_UINT8 *cfl_buffer_getDataPtr(const CFL_BUFFERP buffer);

/**
 * @brief Gets a pointer to the current read/write position.
 * @param buffer Pointer to the buffer.
 * @return Pointer to the current position in the data.
 */
extern CFL_UINT8 *cfl_buffer_positionPtr(CFL_BUFFERP buffer);

/**
 * @brief Sets the byte order for multi-byte operations.
 * @param buffer Pointer to the buffer.
 * @param bigEndian CFL_TRUE for big endian, CFL_FALSE for little endian.
 */
extern void cfl_buffer_setBigEndian(CFL_BUFFERP buffer, CFL_BOOL bigEndian);

/**
 * @brief Checks if buffer uses big endian byte order.
 * @param buffer Pointer to the buffer.
 * @return CFL_TRUE if big endian, CFL_FALSE if little endian.
 */
extern CFL_BOOL cfl_buffer_isBigEndian(const CFL_BUFFERP buffer);

/**
 * @brief Returns the current data length.
 * @param buffer Pointer to the buffer.
 * @return Length of data in bytes.
 */
extern CFL_UINT32 cfl_buffer_length(const CFL_BUFFERP buffer);

/**
 * @brief Sets the data length.
 * @param buffer Pointer to the buffer.
 * @param newLen New length in bytes.
 * @return CFL_TRUE on success, CFL_FALSE on failure.
 */
extern CFL_BOOL cfl_buffer_setLength(CFL_BUFFERP buffer, CFL_UINT32 newLen);

/**
 * @brief Flips the buffer for reading (sets length to position, position to 0).
 * @param buffer Pointer to the buffer.
 */
extern void cfl_buffer_flip(CFL_BUFFERP buffer);

/**
 * @brief Resets the buffer to initial state (position = 0, length = 0).
 * @param buffer Pointer to the buffer.
 */
extern void cfl_buffer_reset(CFL_BUFFERP buffer);

/**
 * @brief Returns the current read/write position.
 * @param buffer Pointer to the buffer.
 * @return Current position in bytes.
 */
extern CFL_UINT32 cfl_buffer_position(const CFL_BUFFERP buffer);

/**
 * @brief Sets the read/write position.
 * @param buffer Pointer to the buffer.
 * @param newPos New position in bytes.
 */
extern void cfl_buffer_setPosition(CFL_BUFFERP buffer, CFL_UINT32 newPos);

/**
 * @brief Advances the position by a number of bytes.
 * @param buffer Pointer to the buffer.
 * @param skip Number of bytes to skip.
 */
extern void cfl_buffer_skip(CFL_BUFFERP buffer, CFL_INT32 skip);

/**
 * @brief Rewinds the buffer (sets position to 0).
 * @param buffer Pointer to the buffer.
 */
extern void cfl_buffer_rewind(CFL_BUFFERP buffer);

/**
 * @brief Compacts the buffer by removing already-read data.
 * @param buffer Pointer to the buffer.
 */
extern void cfl_buffer_compact(CFL_BUFFERP buffer);

/**
 * @brief Returns the current capacity.
 * @param buffer Pointer to the buffer.
 * @return Capacity in bytes.
 */
extern CFL_UINT32 cfl_buffer_capacity(const CFL_BUFFERP buffer);

/**
 * @brief Sets the buffer capacity.
 * @param buffer Pointer to the buffer.
 * @param newCapacity New capacity in bytes.
 * @return CFL_TRUE on success, CFL_FALSE on failure.
 */
extern CFL_BOOL cfl_buffer_setCapacity(CFL_BUFFERP buffer,
                                       CFL_UINT32 newCapacity);

/** @brief Writes a boolean value. */
extern CFL_BOOL cfl_buffer_putBoolean(CFL_BUFFERP buffer, CFL_BOOL value);
/** @brief Reads a boolean value. */
extern CFL_BOOL cfl_buffer_getBoolean(CFL_BUFFERP buffer);
/** @brief Peeks a boolean value without advancing position. */
extern CFL_BOOL cfl_buffer_peekBoolean(CFL_BUFFERP buffer);

/** @brief Writes a signed 8-bit integer. */
extern CFL_BOOL cfl_buffer_putInt8(CFL_BUFFERP buffer, CFL_INT8 value);
/** @brief Reads a signed 8-bit integer. */
extern CFL_INT8 cfl_buffer_getInt8(CFL_BUFFERP buffer);
/** @brief Peeks a signed 8-bit integer. */
extern CFL_INT8 cfl_buffer_peekInt8(CFL_BUFFERP buffer);

/** @brief Writes a signed 16-bit integer. */
extern CFL_BOOL cfl_buffer_putInt16(CFL_BUFFERP buffer, CFL_INT16 value);
/** @brief Reads a signed 16-bit integer. */
extern CFL_INT16 cfl_buffer_getInt16(CFL_BUFFERP buffer);
/** @brief Peeks a signed 16-bit integer. */
extern CFL_INT16 cfl_buffer_peekInt16(CFL_BUFFERP buffer);

/** @brief Writes a signed 32-bit integer. */
extern CFL_BOOL cfl_buffer_putInt32(CFL_BUFFERP buffer, CFL_INT32 value);
/** @brief Reads a signed 32-bit integer. */
extern CFL_INT32 cfl_buffer_getInt32(CFL_BUFFERP buffer);
/** @brief Peeks a signed 32-bit integer. */
extern CFL_INT32 cfl_buffer_peekInt32(CFL_BUFFERP buffer);

/** @brief Writes a signed 64-bit integer. */
extern CFL_BOOL cfl_buffer_putInt64(CFL_BUFFERP buffer, CFL_INT64 value);
/** @brief Reads a signed 64-bit integer. */
extern CFL_INT64 cfl_buffer_getInt64(CFL_BUFFERP buffer);
/** @brief Peeks a signed 64-bit integer. */
extern CFL_INT64 cfl_buffer_peekInt64(CFL_BUFFERP buffer);

/** @brief Writes an unsigned 8-bit integer. */
extern CFL_BOOL cfl_buffer_putUInt8(CFL_BUFFERP buffer, CFL_UINT8 value);
/** @brief Reads an unsigned 8-bit integer. */
extern CFL_UINT8 cfl_buffer_getUInt8(CFL_BUFFERP buffer);
/** @brief Peeks an unsigned 8-bit integer. */
extern CFL_UINT8 cfl_buffer_peekUInt8(CFL_BUFFERP buffer);

/** @brief Writes an unsigned 16-bit integer. */
extern CFL_BOOL cfl_buffer_putUInt16(CFL_BUFFERP buffer, CFL_UINT16 value);
/** @brief Reads an unsigned 16-bit integer. */
extern CFL_UINT16 cfl_buffer_getUInt16(CFL_BUFFERP buffer);
/** @brief Peeks an unsigned 16-bit integer. */
extern CFL_UINT16 cfl_buffer_peekUInt16(CFL_BUFFERP buffer);

/** @brief Writes an unsigned 32-bit integer. */
extern CFL_BOOL cfl_buffer_putUInt32(CFL_BUFFERP buffer, CFL_UINT32 value);
/** @brief Reads an unsigned 32-bit integer. */
extern CFL_UINT32 cfl_buffer_getUInt32(CFL_BUFFERP buffer);
/** @brief Peeks an unsigned 32-bit integer. */
extern CFL_UINT32 cfl_buffer_peekUInt32(CFL_BUFFERP buffer);

/** @brief Writes an unsigned 64-bit integer. */
extern CFL_BOOL cfl_buffer_putUInt64(CFL_BUFFERP buffer, CFL_UINT64 value);
/** @brief Reads an unsigned 64-bit integer. */
extern CFL_UINT64 cfl_buffer_getUInt64(CFL_BUFFERP buffer);
/** @brief Peeks an unsigned 64-bit integer. */
extern CFL_UINT64 cfl_buffer_peekUInt64(CFL_BUFFERP buffer);

/** @brief Writes a float. */
extern CFL_BOOL cfl_buffer_putFloat(CFL_BUFFERP buffer, float value);
/** @brief Reads a float. */
extern float cfl_buffer_getFloat(CFL_BUFFERP buffer);
/** @brief Peeks a float. */
extern float cfl_buffer_peekFloat(CFL_BUFFERP buffer);

/** @brief Writes a double. */
extern CFL_BOOL cfl_buffer_putDouble(CFL_BUFFERP buffer, double value);
/** @brief Reads a double. */
extern double cfl_buffer_getDouble(CFL_BUFFERP buffer);
/** @brief Peeks a double. */
extern double cfl_buffer_peekDouble(CFL_BUFFERP buffer);

/** @brief Reads a string from buffer. */
extern CFL_STRP cfl_buffer_getString(CFL_BUFFERP buffer);
/** @brief Gets length of next string without reading it. */
extern CFL_UINT32 cfl_buffer_getStringLength(CFL_BUFFERP buffer);
/** @brief Copies a string from buffer to destination. */
extern void cfl_buffer_copyString(CFL_BUFFERP buffer, CFL_STRP destStr);
/** @brief Copies a string of specific length. */
extern void cfl_buffer_copyStringLen(CFL_BUFFERP buffer, CFL_STRP destStr,
                                     CFL_UINT32 len);
/** @brief Writes a string with specific length. */
extern CFL_BOOL cfl_buffer_putStringLen(CFL_BUFFERP buffer, CFL_STRP value,
                                        CFL_UINT32 len);
/** @brief Writes a string. */
extern CFL_BOOL cfl_buffer_putString(CFL_BUFFERP buffer, CFL_STRP value);

/** @brief Gets length of next char array. */
extern CFL_UINT32 cfl_buffer_getCharArrayLength(CFL_BUFFERP buffer);
/** @brief Reads a char array from buffer. */
extern char *cfl_buffer_getCharArray(CFL_BUFFERP buffer);
/** @brief Copies a char array to destination. */
extern void cfl_buffer_copyCharArray(CFL_BUFFERP buffer, char *destStr);
/** @brief Copies a char array with specific length. */
extern void cfl_buffer_copyCharArrayLen(CFL_BUFFERP buffer, char *destStr,
                                        CFL_UINT32 len);
/** @brief Writes a char array with specific length. */
extern CFL_BOOL cfl_buffer_putCharArrayLen(CFL_BUFFERP buffer,
                                           const char *value, CFL_UINT32 len);
/** @brief Writes a null-terminated char array. */
extern CFL_BOOL cfl_buffer_putCharArray(CFL_BUFFERP buffer, const char *value);

/** @brief Reads a date from buffer. */
extern void cfl_buffer_getDate(CFL_BUFFERP buffer, CFL_DATEP date);
/** @brief Writes a date to buffer. */
extern CFL_BOOL cfl_buffer_putDate(CFL_BUFFERP buffer, CFL_DATE value);
/** @brief Writes a date pointer to buffer. */
extern CFL_BOOL cfl_buffer_putDatePtr(CFL_BUFFERP buffer, CFL_DATEP value);

/** @brief Gets raw bytes from buffer. */
extern CFL_UINT8 *cfl_buffer_get(CFL_BUFFERP buffer, CFL_UINT32 size);
/** @brief Copies raw bytes from buffer. */
extern void cfl_buffer_copy(CFL_BUFFERP buffer, CFL_UINT8 *dest,
                            CFL_UINT32 size);
/** @brief Writes raw bytes to buffer. */
extern CFL_BOOL cfl_buffer_put(CFL_BUFFERP buffer, void *value,
                               CFL_UINT32 size);
/** @brief Appends another buffer's contents. */
extern CFL_BOOL cfl_buffer_putBuffer(CFL_BUFFERP buffer, CFL_BUFFERP other);
/** @brief Appends a specific size from another buffer. */
extern CFL_BOOL cfl_buffer_putBufferSize(CFL_BUFFERP buffer, CFL_BUFFERP other,
                                         CFL_UINT32 size);

/**
 * @brief Returns remaining bytes (length - position).
 * @param buffer Pointer to the buffer.
 * @return Number of remaining bytes.
 */
extern CFL_UINT32 cfl_buffer_remaining(const CFL_BUFFERP buffer);

/**
 * @brief Checks if buffer has enough remaining bytes.
 * @param buffer Pointer to the buffer.
 * @param need Number of bytes needed.
 * @return CFL_TRUE if enough, CFL_FALSE otherwise.
 */
extern CFL_BOOL cfl_buffer_haveEnough(const CFL_BUFFERP buffer,
                                      CFL_UINT32 need);

/**
 * @brief Writes formatted data to buffer.
 * @param buffer Pointer to the buffer.
 * @param format Printf-style format string.
 * @return CFL_TRUE on success, CFL_FALSE on failure.
 */
extern CFL_BOOL cfl_buffer_putFormat(CFL_BUFFERP buffer, const char *format,
                                     ...);

/**
 * @brief Moves all data from one buffer to another.
 * @param fromBuffer Source buffer.
 * @param toBuffer Destination buffer.
 * @return CFL_TRUE on success, CFL_FALSE on failure.
 */
extern CFL_BOOL cfl_buffer_moveTo(CFL_BUFFERP fromBuffer, CFL_BUFFERP toBuffer);

#ifdef __cplusplus
}
#endif

#endif