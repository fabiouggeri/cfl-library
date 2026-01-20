/**
 * @file cfl_error.h
 * @brief Error handling structure and utilities.
 *
 * This module provides an error structure for storing and managing
 * error information including error codes, types, messages, and user data.
 */

#ifndef CFL_ERROR_H_

#define CFL_ERROR_H_

#include "cfl_str.h"
#include "cfl_types.h"


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Error structure for storing error information.
 */
typedef struct _CFL_ERROR {
  CFL_STRP message; /**< Error message string */
  void *userData;   /**< Optional user-defined data associated with the error */
  CFL_INT32 code;   /**< Error code */
  CFL_UINT8 type;   /**< Error type/category */
  CFL_BOOL allocated; /**< Whether structure was dynamically allocated */
} CFL_ERROR, *CFL_ERRORP;

/**
 * @brief Creates a new error structure.
 * @return Pointer to the new error structure, or NULL if allocation fails.
 */
extern CFL_ERRORP cfl_error_new(void);

/**
 * @brief Frees an error structure and its resources.
 * @param pError Pointer to the error structure to free.
 */
extern void cfl_error_free(CFL_ERRORP pError);

/**
 * @brief Initializes an error structure with default values.
 * @param pError Pointer to the error structure to initialize.
 */
extern void cfl_error_init(CFL_ERRORP pError);

/**
 * @brief Gets the error type.
 * @param pError Pointer to the error structure.
 * @return The error type value.
 */
extern CFL_UINT8 cfl_error_getType(const CFL_ERRORP pError);

/**
 * @brief Sets the error type.
 * @param pError Pointer to the error structure.
 * @param type The error type value to set.
 */
extern void cfl_error_setType(CFL_ERRORP pError, CFL_UINT8 type);

/**
 * @brief Gets the error code.
 * @param pError Pointer to the error structure.
 * @return The error code.
 */
extern CFL_UINT32 cfl_error_getCode(const CFL_ERRORP pError);

/**
 * @brief Sets the error code.
 * @param pError Pointer to the error structure.
 * @param code The error code to set.
 */
extern void cfl_error_setCode(CFL_ERRORP pError, CFL_UINT32 code);

/**
 * @brief Gets the error message as a CFL_STRP.
 * @param pError Pointer to the error structure.
 * @return Pointer to the error message string structure.
 */
extern CFL_STRP cfl_error_getMessage(const CFL_ERRORP pError);

/**
 * @brief Gets the error message as a C-style string.
 * @param pError Pointer to the error structure.
 * @return Pointer to the null-terminated error message string.
 */
extern const char *cfl_error_getMessageStr(const CFL_ERRORP pError);

/**
 * @brief Sets the error message.
 * @param pError Pointer to the error structure.
 * @param message The error message string to set.
 */
extern void cfl_error_setMessage(CFL_ERRORP pError, const char *message);

/**
 * @brief Gets the user data associated with the error.
 * @param pError Pointer to the error structure.
 * @return Pointer to the user data, or NULL if not set.
 */
extern void *cfl_error_getUserData(const CFL_ERRORP pError);

/**
 * @brief Sets the user data associated with the error.
 * @param pError Pointer to the error structure.
 * @param userData Pointer to user-defined data to associate with the error.
 */
extern void cfl_error_setUserData(CFL_ERRORP pError, void *userData);

/**
 * @brief Clears the error structure, resetting all fields to defaults.
 * @param pError Pointer to the error structure to clear.
 */
extern void cfl_error_clear(CFL_ERRORP pError);

#ifdef __cplusplus
}
#endif

#endif
