/**
 * @file cfl_process.h
 * @brief Process management functions.
 */

#ifndef _CFL_PROCESS_H_

#define _CFL_PROCESS_H_

#include "cfl_types.h"

/**
 * @brief Gets the unique ID of the current process.
 * @return The process ID (PID).
 */
extern CFL_INT64 cfl_process_getId(void);

#endif