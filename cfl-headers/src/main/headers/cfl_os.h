/**
 * @file cfl_os.h
 * @brief Operating System detection and abstraction macros.
 *
 * This header detects the target operating system and architecture,
 * defining macros to allow conditional compilation for platform-specific code.
 */

#ifndef _CFL_OS_H_

#define _CFL_OS_H_

#if defined(_WIN32)
/** @brief Defined if compiling for Windows */
#define CFL_OS_WINDOWS
#if defined(_WIN64)
/** @brief Defined if compiling for 64-bit architecture */
#define CFL_ARCH_64
#else
/** @brief Defined if compiling for 32-bit architecture */
#define CFL_ARCH_32
#endif
#elif defined(__linux__)
#include <errno.h>
#include <pthread.h>
#include <sched.h>
/** @brief Defined if compiling for Linux */
#define CFL_OS_LINUX
#if defined(__LP64__) || defined(__x86_64__) || defined(__ppc64__)
/** @brief Defined if compiling for 64-bit architecture */
#define CFL_ARCH_64
#else
/** @brief Defined if compiling for 32-bit architecture */
#define CFL_ARCH_32
#endif
#endif

#endif
