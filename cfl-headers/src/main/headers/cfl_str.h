/**
 * @file cfl_str.h
 * @brief Dynamic string implementation with utility functions.
 *
 * This module provides a dynamic string data structure with functions for
 * creation, manipulation, formatting, comparison, and searching.
 */

#ifndef CFL_STR_H_

#define CFL_STR_H_

#include "cfl_types.h"
#include <stdarg.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Safely frees a string and sets pointer to NULL.
 * @param s The string pointer to free.
 */
#define CFL_STR_FREE(s)                                                        \
  if (s != NULL) {                                                             \
    cfl_str_free(s);                                                           \
    s = NULL;                                                                  \
  }

/**
 * @brief Appends a compile-time constant string.
 * @param s The destination string.
 * @param c The compile-time constant string to append.
 */
#define CFL_STR_APPEND_CONST(s, c) cfl_str_appendLen(s, c, (int)sizeof(c) - 1)

/**
 * @brief Sets string to a compile-time constant.
 * @param s The destination string.
 * @param c The compile-time constant string to set.
 */
#define CFL_STR_SET_CONST(s, c) cfl_str_setConstLen(s, c, (int)sizeof(c) - 1)

/** @brief Checks if character is uppercase (A-Z). */
#define CFL_ISUPPER(c) ((c) >= 'A' && (c) <= 'Z')
/** @brief Checks if character is lowercase (a-z). */
#define CFL_ISLOWER(c) ((c) >= 'a' && (c) <= 'z')
/** @brief Checks if character is a digit (0-9). */
#define CFL_ISDIGIT(c) ((c) >= '0' && (c) <= '9')
/** @brief Converts lowercase character to uppercase. */
#define CFL_TOUPPER(c) ((c) - ('a' - 'A'))
/** @brief Converts uppercase character to lowercase. */
#define CFL_TOLOWER(c) ((c) + ('a' - 'A'))
/** @brief Checks if character is whitespace. */
#define CFL_ISSPACE(c) isspace(c)

/** @brief Initializer for an empty string. */
#define CFL_STR_EMPTY {"", 0, 1, 0, CFL_FALSE, CFL_FALSE}
/** @brief Initializer for a constant string literal. */
#define CFL_STR_CONST(s) {s, sizeof(s) - 1, sizeof(s), 0, CFL_FALSE, CFL_FALSE}

/**
 * @brief Dynamic string structure.
 *
 * This structure represents a dynamic string with automatic memory management.
 * It supports both dynamically allocated data and references to constant
 * strings.
 */
typedef struct _CFL_STR {
  char *data;          /**< Pointer to string data */
  CFL_UINT32 length;   /**< Current string length (excluding null terminator) */
  CFL_UINT32 dataSize; /**< Allocated buffer size */
  CFL_UINT32 hashValue; /**< Cached hash value (0 if not computed) */
  CFL_BOOL isVarData;   /**< True if data was dynamically allocated */
  CFL_BOOL isAllocated; /**< True if structure was dynamically allocated */
} CFL_STR, *CFL_STRP;

/**
 * @brief Initializes a string with default values.
 *
 * Sets the string to an empty state with zero size, length and hash value.
 * The data pointer is set to an empty string literal.
 *
 * @param str Pointer to the CFL_STR structure to initialize.
 *            If NULL, the function returns without doing anything.
 */
extern void cfl_str_init(CFL_STRP str);

/**
 * @brief Initializes a string with specified capacity.
 *
 * Allocates memory for the string data if the initial capacity is greater than
 * 0. If memory allocation fails or initial capacity is 0, the string will be
 * initialized as an empty constant string.
 *
 * @param str Pointer to the CFL_STR structure to be initialized.
 *            If NULL, function returns without effect.
 * @param iniCapacity Initial capacity for the string buffer.
 *                    If 0, no memory is allocated.
 *
 * @note The actual allocated size will be iniCapacity + 1 to accommodate the
 * null terminator.
 * @note If memory allocation fails, the string will be initialized as an empty
 * constant string.
 */
extern void cfl_str_initCapacity(CFL_STRP str, CFL_UINT32 iniCapacity);

/**
 * @brief Initializes a string with a constant buffer of specified length.
 *
 * Initializes a string structure with a provided buffer, setting it up
 * as a constant (non-allocated) string. If the buffer is NULL or length is 0,
 * the string is initialized as empty.
 *
 * @param str Pointer to the CFL_STR structure to be initialized.
 *            If NULL, the function returns without doing anything.
 * @param buffer Constant character buffer to be used as string data.
 * @param len Length of the buffer (not including null terminator).
 *
 * @note The buffer is used directly without copying, so it must remain valid
 *       for the lifetime of the string.
 */
extern void cfl_str_initConstLen(CFL_STRP str, const char *buffer,
                                 CFL_UINT32 len);

/**
 * @brief Initializes a string with a constant buffer.
 *
 * Initializes a CFL string structure with a constant character buffer.
 * If the buffer is NULL, the string will be initialized with length 0.
 *
 * @param str Pointer to the CFL_STR structure to be initialized.
 *            If NULL, the function returns without doing anything.
 * @param buffer Constant character buffer to initialize the string with.
 *
 * @note The buffer is not copied, the string will reference the original
 * buffer.
 */
extern void cfl_str_initConst(CFL_STRP str, const char *buffer);

/**
 * @brief Initializes a string by copying a buffer.
 *
 * Initializes a CFL string structure with the contents of the provided buffer.
 * If the buffer is NULL or empty, the string will be initialized as empty.
 * The function allocates memory for the string data if needed.
 *
 * @param str Pointer to the CFL_STR structure to initialize.
 *            If NULL, the function returns without doing anything.
 * @param buffer Source buffer containing the initial string value (can be
 * NULL).
 *
 * @note If memory allocation fails, the string will be initialized as empty.
 * @note The string takes ownership of the allocated memory if successful.
 */
extern void cfl_str_initValue(CFL_STRP str, const char *buffer);

/**
 * @brief Creates a new empty string with specified capacity.
 *
 * Allocates memory for a new string structure and its data buffer.
 * The string is initialized as empty but with space to store the specified
 * number of characters plus the null terminator.
 *
 * @param iniCapacity Initial capacity of the string buffer (number of chars
 * that can be stored). If 0, creates an empty string with minimum allocation.
 *
 * @return A pointer to the newly created string structure (CFL_STRP),
 *         or NULL if memory allocation fails.
 *
 * @note The actual allocated size will be iniCapacity + 1 to accommodate the
 * null terminator.
 * @note The returned string is marked as allocated and variable data.
 */
extern CFL_STRP cfl_str_new(CFL_UINT32 iniCapacity);

/**
 * @brief Creates a new string by copying a buffer.
 *
 * Allocates memory for a new string structure and copies the content
 * from the provided buffer. The resulting string is null-terminated.
 *
 * @param buffer The source C-style string buffer to create the CFL string from.
 *               If NULL, an empty string will be created.
 *
 * @return A new CFL string pointer (CFL_STRP) containing the contents of the
 * buffer. The returned string must be freed using cfl_str_free when no longer
 * needed. Returns NULL if memory allocation fails.
 */
extern CFL_STRP cfl_str_newBuffer(const char *buffer);

/**
 * @brief Creates a new string by copying a buffer with length.
 *
 * Allocates memory for a new string structure and copies the content
 * from the provided buffer up to the specified length.
 * The resulting string is null-terminated.
 *
 * @param buffer The source buffer containing the string content to copy.
 *               If NULL or empty, returns an empty string.
 * @param len The number of characters to copy from the buffer.
 *            If 0, returns an empty string.
 *
 * @return A pointer to the newly created CFL_STR structure, or NULL if
 *         memory allocation fails. The returned string must be freed
 *         using cfl_str_free() when no longer needed.
 */
extern CFL_STRP cfl_str_newBufferLen(const char *buffer, CFL_UINT32 len);

/**
 * @brief Creates a new string referencing a constant buffer.
 *
 * Creates a new CFL_STR structure and initializes it with the provided buffer.
 * The buffer is not copied but directly referenced.
 *
 * @param buffer Pointer to the constant character buffer to reference.
 *               If NULL, an empty string will be created.
 *
 * @return A pointer to the newly created CFL_STR structure, or NULL if memory
 *         allocation fails.
 *
 * @note The buffer is not copied, the string object will reference the original
 * buffer. The caller must ensure the buffer remains valid for the lifetime of
 * the string.
 */
extern CFL_STRP cfl_str_newConst(const char *buffer);

/**
 * @brief Creates a new string referencing a constant buffer with length.
 *
 * Creates a new CFL_STR structure and initializes it with the provided
 * buffer and length. The buffer is not copied but directly referenced.
 *
 * @param buffer Pointer to the constant character buffer to reference.
 *               If NULL or length is 0, an empty string will be created.
 * @param len Length of the buffer in characters.
 *
 * @return A pointer to the newly created CFL_STR structure, or NULL if memory
 *         allocation fails.
 *
 * @note The buffer is not copied, the string object will reference the original
 * buffer. The caller must ensure the buffer remains valid for the lifetime of
 * the string.
 */
extern CFL_STRP cfl_str_newConstLen(const char *buffer, CFL_UINT32 len);

/**
 * @brief Creates a new string by copying another string.
 *
 * Creates a new CFL_STR structure and initializes it with the contents
 * of the provided string. If the source string contains variable data,
 * it allocates new memory and copies the data. Otherwise, it shares the data
 * pointer.
 *
 * @param str Pointer to the source string structure to copy from.
 *            If NULL or empty, returns a new empty string.
 *
 * @return Pointer to the newly created string structure (CFL_STRP),
 *         or NULL if memory allocation fails.
 *
 * @note The function handles both variable and fixed data strings differently:
 *       - For variable data: Allocates new memory and copies the content.
 *       - For fixed data: Shares the data pointer with the source string.
 */
extern CFL_STRP cfl_str_newStr(CFL_STRP str);

/**
 * @brief Frees a string and its data.
 *
 * Deallocates memory used by a CFL_STRP string object.
 * If the string has variable data allocated, it frees that memory first.
 * If the string object itself was dynamically allocated, it frees the object as
 * well.
 *
 * @param str Pointer to the string object to be freed.
 *            If NULL, the function returns without doing anything.
 */
extern void cfl_str_free(CFL_STRP str);

/**
 * @brief Appends one or more null-terminated strings.
 *
 * Concatenates multiple strings to an existing CFL_STR object.
 * If the input string object is NULL, a new string object is created with
 * default capacity. The function ensures there is enough capacity to store all
 * appended strings.
 *
 * @param str The target CFL_STR object to append to. If NULL, a new string is
 * created.
 * @param buffer The first string to append, followed by additional string
 * arguments.
 * @param ... Variable number of string arguments (must be char*), terminated by
 * NULL.
 *
 * @return The resulting CFL_STR object after appending all strings.
 *
 * @note The function resets the hash value of the resulting string to 0.
 * @note All variable arguments must be of type char* and the list must be
 * NULL-terminated.
 *
 * @code
 * CFL_STRP result = cfl_str_append(str, "Hello", " ", "World", NULL);
 * @endcode
 */
extern CFL_STRP cfl_str_append(CFL_STRP str, const char *buffer, ...);

/**
 * @brief Appends a single character.
 *
 * Appends a character to the end of a string.
 * If the input string is NULL, creates a new string with default capacity.
 *
 * @param str The string to append to, or NULL to create a new string.
 * @param c The character to append.
 *
 * @return The resulting string, or NULL if memory allocation fails.
 *
 * @note If the input string needs to grow to accommodate the new character,
 *       it will be reallocated with increased capacity.
 * @note The hash value of the string is invalidated (set to 0) after appending.
 */
extern CFL_STRP cfl_str_appendChar(CFL_STRP str, char c);

/**
 * @brief Appends a buffer of specified length.
 *
 * Appends a buffer of a given length to a string.
 * If the input string is NULL and the buffer is valid, creates a new string
 * with the buffer content. If the input string is NULL and the buffer is
 * invalid, creates a new empty string with default capacity. If the input
 * string exists and the buffer is valid, appends the buffer content to the
 * existing string.
 *
 * @param str The string to append to, or NULL to create a new string.
 * @param buffer The buffer containing the characters to append.
 * @param len The number of characters to append from the buffer.
 *
 * @return The resulting string (either new or modified), or NULL if memory
 * allocation fails.
 */
extern CFL_STRP cfl_str_appendLen(CFL_STRP str, const char *buffer,
                                  CFL_UINT32 len);

/**
 * @brief Appends another string.
 *
 * Appends a string to another string.
 *
 * @param str The destination string to append to. If NULL, a new string will be
 * created.
 * @param strAppend The source string to append. If NULL or empty, no append
 * operation is performed.
 *
 * @return The resulting string after append operation. If str is NULL and
 * strAppend is NULL/empty, returns a new empty string with default capacity.
 *
 * @note If the destination string doesn't have enough capacity, it will be
 * automatically resized. The hash value of the destination string is reset to 0
 * after append.
 */
extern CFL_STRP cfl_str_appendStr(CFL_STRP str, CFL_STRP strAppend);

/**
 * @brief Appends formatted text using va_list.
 *
 * Appends a formatted string to an existing string using variable arguments.
 *
 * @param str The target string to append to. If NULL, a new string will be
 * created.
 * @param format The format string specifying how to format the variable
 * arguments.
 * @param varArgs A va_list containing the variable arguments to format.
 *
 * @return Returns the resulting string pointer (CFL_STRP). If input str was
 * NULL, returns a newly allocated string. If an error occurs during allocation
 *         or formatting, returns NULL or the original string unchanged.
 *
 * @note The function first calculates required length, ensures capacity, then
 *       performs the actual formatting. The string's hash value is reset to 0
 *       after modification.
 */
extern CFL_STRP cfl_str_appendFormatArgs(CFL_STRP str, const char *format,
                                         va_list varArgs);

/**
 * @brief Appends formatted text.
 *
 * Appends text formatted according to the format string to the given CFL_STRP
 * string. The format string follows the same rules as printf.
 *
 * @param str The string to append to. If NULL, a new string will be created.
 * @param format The format string following printf conventions.
 * @param ... Variable arguments corresponding to format specifiers.
 *
 * @return The modified string (CFL_STRP).
 *
 * @note The function uses variable arguments internally and properly handles
 * their cleanup.
 */
extern CFL_STRP cfl_str_appendFormat(CFL_STRP str, const char *format, ...);

/**
 * @brief Gets a pointer to the string data.
 *
 * Returns a pointer to the internal character array of a string.
 *
 * @param str The CFL_STRP (string pointer) to get the data from.
 *
 * @return Pointer to the internal character array.
 *
 * @warning The returned pointer should not be modified directly as it points to
 *          the internal data of the string structure.
 */
extern char *cfl_str_getPtr(const CFL_STRP str);

/**
 * @brief Gets a pointer to a character at index.
 *
 * Returns a pointer to the character at the specified index in the string.
 *
 * @param str The string to get the character from.
 * @param index The index of the character to get.
 *
 * @return A pointer to the character at the specified index, or NULL if the
 * index is out of bounds.
 */
extern char *cfl_str_getPtrAt(const CFL_STRP str, CFL_UINT32 index);

/**
 * @brief Returns the string length (deprecated, use cfl_str_length).
 *
 * @param str Pointer to a CFL string structure.
 *
 * @return The length of the string as CFL_UINT32.
 *
 * @deprecated Use cfl_str_length() instead.
 */
extern CFL_UINT32 cfl_str_getLength(const CFL_STRP str);

/**
 * @brief Returns the string length.
 *
 * @param str Pointer to a CFL string structure.
 *
 * @return The length of the string as CFL_UINT32 (not including null
 * terminator).
 */
extern CFL_UINT32 cfl_str_length(const CFL_STRP str);

/**
 * @brief Sets the string length.
 *
 * Sets the length of a string to the specified value.
 * If the new length is greater than the current length, the additional space
 * is filled with spaces. If the new length is smaller, the string is truncated.
 *
 * @param str Pointer to the string structure to modify.
 * @param newLen The new desired length for the string.
 *
 * @note The function ensures the target string has enough capacity before
 * setting.
 * @note The hash value is reset to 0 after modification.
 *       A null terminator is always added at the end of the string.
 */
extern void cfl_str_setLength(CFL_STRP str, CFL_UINT32 newLen);

/**
 * @brief Clears the string content.
 *
 * Clears the string by either:
 * - Setting first character to null terminator if string has variable data.
 * - Setting data pointer to empty string and size to 0 if string has fixed
 * data.
 *
 * Also resets the hash value and length to 0.
 *
 * @param str Pointer to the CFL string to be cleared.
 */
extern void cfl_str_clear(CFL_STRP str);

/**
 * @brief Sets string content from another string.
 *
 * Sets the string content of a CFL_STRP from another CFL_STRP source string.
 *
 * @param str The destination string to be modified.
 * @param src The source string to copy from.
 *
 * @return Returns the modified destination string (str).
 *         If src is NULL or empty, sets str to empty string.
 */
extern CFL_STRP cfl_str_setStr(CFL_STRP str, const CFL_STRP src);

/**
 * @brief Sets string content from a buffer.
 *
 * Sets the value of a string from a buffer.
 *
 * @param str The string to set the value of. If NULL, a new string will be
 * created.
 * @param buffer The source buffer containing the string to set.
 *               If NULL, the string will be empty.
 *
 * @return The string pointer with the new value set, or NULL if the operation
 * fails.
 */
extern CFL_STRP cfl_str_setValue(CFL_STRP str, const char *buffer);

/**
 * @brief Sets string content from a buffer with length.
 *
 * Sets or creates a string with the given buffer content.
 *
 * @param str The target string to set. If NULL, a new string will be created.
 * @param buffer The source buffer containing the content to be set.
 * @param len The length of the source buffer.
 *
 * @return The modified string if str was provided, or a new string if str was
 * NULL. Returns NULL if memory allocation fails when creating a new string.
 *
 * @note If buffer is NULL or len is 0, the string will be cleared.
 *       The function ensures the target string has enough capacity before
 * copying. After setting, the string's hash value is reset to 0.
 */
extern CFL_STRP cfl_str_setValueLen(CFL_STRP str, const char *buffer,
                                    CFL_UINT32 len);

/**
 * @brief Sets string to reference a constant buffer.
 *
 * Sets a string constant to an existing CFL_STRP structure by using
 * the provided buffer. The buffer is not copied.
 *
 * @param str Pointer to the CFL_STRP structure to be modified.
 *            If NULL, a new string will be created.
 * @param buffer Constant string buffer to be set. If NULL, the string will be
 * empty.
 *
 * @return Pointer to the modified CFL_STRP structure.
 *
 * @note The buffer must remain valid for the lifetime of the string.
 * @note Any previous variable data in the string will be freed.
 */
extern CFL_STRP cfl_str_setConst(CFL_STRP str, const char *buffer);

/**
 * @brief Sets string to reference a constant buffer with length.
 *
 * Sets or creates a new string with constant length data.
 * If the input string pointer is NULL, creates a new string.
 * If the input string pointer exists, replaces its content.
 * The function takes reference of the buffer memory - it will not copy the
 * data.
 *
 * @param str The string to be modified, or NULL to create a new one.
 * @param buffer The buffer containing the string data.
 * @param len The length of the buffer.
 *
 * @return The modified or newly created string.
 *
 * @note If buffer is NULL or len is 0, the string will be set to empty.
 * @note The buffer must remain valid for the lifetime of the string.
 * @note Any previous variable data in the string will be freed.
 */
extern CFL_STRP cfl_str_setConstLen(CFL_STRP str, const char *buffer,
                                    CFL_UINT32 len);

/**
 * @brief Sets string content using formatted text with va_list.
 *
 * Sets the content of a string using a format string and variable arguments.
 * This function formats a string similar to sprintf but manages memory
 * automatically. If the input string pointer is NULL, a new string will be
 * allocated. If the formatting fails, an empty string will be returned.
 *
 * @param str Pointer to the string to be modified, or NULL to create a new
 * string.
 * @param format Format string following printf conventions.
 * @param varArgs Variable argument list containing values to be formatted.
 *
 * @return Pointer to the resulting string (may be newly allocated if input was
 * NULL), or NULL if memory allocation fails.
 *
 * @note The function calculates required space first to ensure proper memory
 * allocation.
 * @note The hashValue of the string is reset to 0 after modification.
 */
extern CFL_STRP cfl_str_setFormatArgs(CFL_STRP str, const char *format,
                                      va_list varArgs);

/**
 * @brief Sets string content using formatted text.
 *
 * Sets the string content using a formatted string with variable arguments.
 *
 * @param str The string object to modify. If NULL, a new string will be
 * created.
 * @param format The format string following printf conventions.
 * @param ... Variable arguments corresponding to the format specifiers.
 *
 * @return The modified string object.
 *
 * @note This function internally uses cfl_str_setFormatArgs to perform the
 * actual formatting.
 */
extern CFL_STRP cfl_str_setFormat(CFL_STRP str, const char *format, ...);

/**
 * @brief Sets a character at specified index.
 *
 * Sets a character at the specified index in a string.
 * If the string is NULL, creates a new string with length index + 1.
 * If the index is beyond the current string length, extends the string to
 * accommodate the new index.
 *
 * @param str The string to modify. If NULL, a new string will be created.
 * @param index The position where to set the character.
 * @param c The character to set at the specified position.
 *
 * @return The modified string (or newly created string if input was NULL).
 */
extern CFL_STRP cfl_str_setChar(CFL_STRP str, CFL_UINT32 index, char c);

/**
 * @brief Checks if string starts with another string.
 *
 * @param str The string to check.
 * @param strStart The string to find at the start.
 *
 * @return CFL_TRUE if str starts with strStart, CFL_FALSE otherwise.
 */
extern CFL_BOOL cfl_str_startsWith(const CFL_STRP str, const CFL_STRP strStart);

/**
 * @brief Checks if string starts with another string (case-insensitive).
 *
 * @param str The string to check.
 * @param strStart The string to find at the start.
 *
 * @return CFL_TRUE if str starts with strStart (ignoring case), CFL_FALSE
 * otherwise.
 */
extern CFL_BOOL cfl_str_startsWithIgnoreCase(const CFL_STRP str,
                                             const CFL_STRP strStart);

/**
 * @brief Checks if string starts with a buffer.
 *
 * @param str The string to check.
 * @param buffer The C-style string buffer to find at the start.
 *
 * @return CFL_TRUE if str starts with buffer, CFL_FALSE otherwise.
 */
extern CFL_BOOL cfl_str_bufferStartsWith(const CFL_STRP str,
                                         const char *buffer);

/**
 * @brief Checks if string starts with a buffer (case-insensitive).
 *
 * @param str The string to check.
 * @param buffer The C-style string buffer to find at the start.
 *
 * @return CFL_TRUE if str starts with buffer (ignoring case), CFL_FALSE
 * otherwise.
 */
extern CFL_BOOL cfl_str_bufferStartsWithIgnoreCase(const CFL_STRP str,
                                                   const char *buffer);

/**
 * @brief Checks if two strings are equal.
 *
 * Performs an exact comparison between two strings.
 *
 * @param str1 First string to compare.
 * @param str2 Second string to compare.
 *
 * @return CFL_TRUE if strings are equal, CFL_FALSE otherwise.
 */
extern CFL_BOOL cfl_str_equals(const CFL_STRP str1, const CFL_STRP str2);

/**
 * @brief Checks if two strings are equal (case-insensitive).
 *
 * Performs a case-insensitive comparison between two strings.
 *
 * @param str1 First string to compare.
 * @param str2 Second string to compare.
 *
 * @return CFL_TRUE if strings are equal (ignoring case), CFL_FALSE otherwise.
 */
extern CFL_BOOL cfl_str_equalsIgnoreCase(const CFL_STRP str1,
                                         const CFL_STRP str2);

/**
 * @brief Compares two strings lexicographically.
 *
 * @param str1 First string to compare.
 * @param str2 Second string to compare.
 * @param bExact If CFL_TRUE, performs exact comparison.
 *               If CFL_FALSE, treats strings as equal if one is a prefix of the
 * other.
 *
 * @return 0 if strings are equal (or one is prefix of other when bExact is
 * CFL_FALSE), -1 if str1 is lexicographically less than str2, 1 if str1 is
 * lexicographically greater than str2.
 *
 * @note If str1 and str2 point to the same string or have the same data
 * pointer, they are considered equal and function returns 0 immediately.
 */
extern CFL_INT16 cfl_str_compare(const CFL_STRP str1, const CFL_STRP str2,
                                 CFL_BOOL bExact);

/**
 * @brief Compares two strings (case-insensitive).
 *
 * Compares two strings ignoring case.
 *
 * @param str1 Pointer to the first string to compare.
 * @param str2 Pointer to the second string to compare.
 * @param bExact If CFL_TRUE, performs exact comparison.
 *               If CFL_FALSE, treats strings as equal if one is a prefix of the
 * other.
 *
 * @return 0 if strings are equal or if one is prefix of other (when bExact is
 * CFL_FALSE), -1 if str1 is lexicographically less than str2, 1 if str1 is
 * lexicographically greater than str2.
 *
 * @note The comparison is case-insensitive (e.g., 'A' equals 'a').
 * @note If str1 and str2 point to the same string or same data, returns 0
 * immediately.
 */
extern CFL_INT16 cfl_str_compareIgnoreCase(const CFL_STRP str1,
                                           const CFL_STRP str2,
                                           CFL_BOOL bExact);

/**
 * @brief Checks if string equals a buffer.
 *
 * Performs an exact comparison between a string and a C-style string buffer.
 *
 * @param str1 The string to compare.
 * @param str2 The C-style string buffer to compare with.
 *
 * @return CFL_TRUE if the string and buffer are equal, CFL_FALSE otherwise.
 */
extern CFL_BOOL cfl_str_bufferEquals(const CFL_STRP str1, const char *str2);

/**
 * @brief Checks if string equals a buffer (case-insensitive).
 *
 * Performs a case-insensitive comparison between a string and a C-style string
 * buffer.
 *
 * @param str1 The string to compare.
 * @param str2 The C-style string buffer to compare with.
 *
 * @return CFL_TRUE if the string and buffer are equal (ignoring case),
 * CFL_FALSE otherwise.
 */
extern CFL_BOOL cfl_str_bufferEqualsIgnoreCase(const CFL_STRP str1,
                                               const char *str2);

/**
 * @brief Compares string with a buffer.
 *
 * Compares a string with a C-style string buffer lexicographically.
 *
 * @param str1 The string to compare.
 * @param str2 The C-style string buffer to compare with.
 * @param bExact If CFL_TRUE, performs exact comparison.
 *               If CFL_FALSE, treats strings as equal if one is a prefix of the
 * other.
 *
 * @return 0 if strings are equal (or one is prefix of other when bExact is
 * CFL_FALSE), -1 if str1 is lexicographically less than str2, 1 if str1 is
 * lexicographically greater than str2.
 */
extern CFL_INT16 cfl_str_bufferCompare(const CFL_STRP str1, const char *str2,
                                       CFL_BOOL bExact);

/**
 * @brief Compares string with a buffer (case-insensitive).
 *
 * Compares a string with a C-style string buffer lexicographically, ignoring
 * case.
 *
 * @param str1 The string to compare.
 * @param str2 The C-style string buffer to compare with.
 * @param bExact If CFL_TRUE, performs exact comparison.
 *               If CFL_FALSE, treats strings as equal if one is a prefix of the
 * other.
 *
 * @return 0 if strings are equal (or one is prefix of other when bExact is
 * CFL_FALSE), -1 if str1 is lexicographically less than str2, 1 if str1 is
 * lexicographically greater than str2.
 */
extern CFL_INT16 cfl_str_bufferCompareIgnoreCase(const CFL_STRP str1,
                                                 const char *str2,
                                                 CFL_BOOL bExact);

/**
 * @brief Computes and caches the hash code.
 *
 * Computes a hash code for the string using a polynomial rolling hash
 * algorithm. The computed value is cached in the hashValue field and reused on
 * subsequent calls.
 *
 * @param str Pointer to the string to compute hash for.
 *
 * @return The computed hash value as CFL_UINT32.
 *
 * @note If the string is empty, returns 0.
 * @note If the hash is already computed (hashValue != 0), returns the cached
 * value.
 */
extern CFL_UINT32 cfl_str_hashCode(CFL_STRP str);

/**
 * @brief Converts string to uppercase.
 *
 * Converts all lowercase characters (a-z) in the string to uppercase (A-Z).
 *
 * @param str Pointer to the string to convert.
 *
 * @return The modified string with all characters in uppercase.
 *
 * @note The hash value is reset to 0 if any characters were modified.
 */
extern CFL_STRP cfl_str_toUpper(CFL_STRP str);

/**
 * @brief Converts string to lowercase.
 *
 * Converts all uppercase characters (A-Z) in the string to lowercase (a-z).
 *
 * @param str Pointer to the string to convert.
 *
 * @return The modified string with all characters in lowercase.
 *
 * @note The hash value is reset to 0 if any characters were modified.
 */
extern CFL_STRP cfl_str_toLower(CFL_STRP str);

/**
 * @brief Removes leading and trailing whitespace.
 *
 * Trims whitespace characters from the beginning and end of the string.
 *
 * @param str Pointer to the string to trim.
 *
 * @return The modified string with leading and trailing whitespace removed.
 *
 * @note The hash value is reset to 0 if any characters were removed.
 */
extern CFL_STRP cfl_str_trim(CFL_STRP str);

/**
 * @brief Checks if string is empty (length == 0).
 *
 * @param str The string to check.
 *
 * @return CFL_TRUE if the string is NULL or has length 0, CFL_FALSE otherwise.
 */
extern CFL_BOOL cfl_str_isEmpty(const CFL_STRP str);

/**
 * @brief Checks if string is blank (empty or only whitespace).
 *
 * @param str The string to check.
 *
 * @return CFL_TRUE if the string is NULL, empty, or contains only whitespace
 * characters, CFL_FALSE otherwise.
 */
extern CFL_BOOL cfl_str_isBlank(const CFL_STRP str);

/**
 * @brief Extracts a substring.
 *
 * Creates a new string containing characters from the specified range.
 *
 * @param str The source string to extract from.
 * @param start The starting index (inclusive).
 * @param end The ending index (exclusive). If greater than string length, uses
 * string length.
 *
 * @return A new string containing the substring, or an empty string if start is
 *         beyond the string length. The returned string must be freed using
 * cfl_str_free().
 */
extern CFL_STRP cfl_str_substr(const CFL_STRP str, CFL_UINT32 start,
                               CFL_UINT32 end);

/**
 * @brief Finds a character starting from an index.
 *
 * Searches for the first occurrence of a character in the string,
 * starting from the specified index.
 *
 * @param str The string to search in.
 * @param search The character to search for.
 * @param start The starting index for the search.
 *
 * @return The index of the first occurrence of the character, or -1 if not
 * found.
 */
extern CFL_INT32 cfl_str_indexOf(const CFL_STRP str, char search,
                                 CFL_UINT32 start);

/**
 * @brief Finds a substring starting from an index.
 *
 * Searches for the first occurrence of a substring (CFL_STRP) in the string,
 * starting from the specified index.
 *
 * @param str The string to search in.
 * @param search The substring to search for.
 * @param start The starting index for the search.
 *
 * @return The index of the first occurrence of the substring, or -1 if not
 * found.
 */
extern CFL_INT32 cfl_str_indexOfStr(const CFL_STRP str, const CFL_STRP search,
                                    CFL_UINT32 start);

/**
 * @brief Finds a buffer starting from an index.
 *
 * Searches for the first occurrence of a buffer in the string,
 * starting from the specified index.
 *
 * @param str The string to search in.
 * @param search The buffer to search for.
 * @param searchLen The length of the search buffer.
 * @param start The starting index for the search.
 *
 * @return The index of the first occurrence of the buffer, or -1 if not found.
 */
extern CFL_INT32 cfl_str_indexOfBuffer(const CFL_STRP str, const char *search,
                                       CFL_UINT32 searchLen, CFL_UINT32 start);

/**
 * @brief Gets character at index (from start).
 *
 * Returns the character at the specified index, counting from the beginning of
 * the string.
 *
 * @param str The string to get the character from.
 * @param index The index of the character to get (0-based).
 *
 * @return The character at the specified index, or '\0' if the index is out of
 * bounds.
 */
extern char cfl_str_charAt(const CFL_STRP str, CFL_UINT32 index);

/**
 * @brief Gets character at index (from end).
 *
 * Returns the character at the specified index, counting from the end of the
 * string.
 *
 * @param str The string to get the character from.
 * @param index The index of the character to get (0 = last character, 1 =
 * second to last, etc.).
 *
 * @return The character at the specified index from the end, or '\0' if the
 * index is out of bounds.
 */
extern char cfl_str_charRAt(const CFL_STRP str, CFL_UINT32 index);

/**
 * @brief Replaces all occurrences of a character.
 *
 * Replaces all occurrences of a specified character with a new character.
 *
 * @param str The string to modify.
 * @param oldChar The character to be replaced.
 * @param newChar The replacement character.
 *
 * @return The number of characters that were replaced.
 */
extern CFL_UINT32 cfl_str_replaceChar(CFL_STRP str, char oldChar, char newChar);

/**
 * @brief Copies a buffer range to destination.
 *
 * Copies a range of characters from a source buffer with known length to a
 * destination string.
 *
 * @param dest The destination string. If NULL, a new string will be created.
 * @param source The source buffer to copy from.
 * @param sourceLen The length of the source buffer.
 * @param start The starting index in the source (inclusive).
 * @param end The ending index in the source (exclusive).
 *
 * @return The destination string containing the copied range.
 *
 * @note If start >= sourceLen, returns an empty string.
 * @note If end < start or end > sourceLen, end is set to sourceLen.
 */
extern CFL_STRP cfl_str_copyBufferLen(CFL_STRP dest, const char *source,
                                      CFL_UINT32 sourceLen, CFL_UINT32 start,
                                      CFL_UINT32 end);

/**
 * @brief Copies a buffer range to destination.
 *
 * Copies a range of characters from a null-terminated source buffer to a
 * destination string.
 *
 * @param dest The destination string. If NULL, a new string will be created.
 * @param source The null-terminated source buffer to copy from.
 * @param start The starting index in the source (inclusive).
 * @param end The ending index in the source (exclusive).
 *
 * @return The destination string containing the copied range.
 */
extern CFL_STRP cfl_str_copyBuffer(CFL_STRP dest, const char *source,
                                   CFL_UINT32 start, CFL_UINT32 end);

/**
 * @brief Copies a string range to destination.
 *
 * Copies a range of characters from a source CFL_STRP to a destination string.
 *
 * @param dest The destination string. If NULL, a new string will be created.
 * @param source The source string to copy from.
 * @param start The starting index in the source (inclusive).
 * @param end The ending index in the source (exclusive).
 *
 * @return The destination string containing the copied range.
 */
extern CFL_STRP cfl_str_copy(CFL_STRP dest, const CFL_STRP source,
                             CFL_UINT32 start, CFL_UINT32 end);

/**
 * @brief Moves source string to destination (transfers ownership).
 *
 * Transfers ownership of the source string's data to the destination string.
 * After this operation, the source string is reset to an empty state.
 * Any previous data in the destination is freed if it was dynamically
 * allocated.
 *
 * @param dest The destination string to receive the data.
 * @param source The source string whose data will be transferred.
 *
 * @return The destination string containing the moved data.
 *
 * @note After the move, the source string is reset to an empty constant string.
 * @note The destination's previous variable data (if any) is freed.
 */
extern CFL_STRP cfl_str_move(CFL_STRP dest, CFL_STRP source);

#ifdef __cplusplus
}
#endif

#endif
