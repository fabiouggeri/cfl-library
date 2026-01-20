/**
 * @file cfl_log.h
 * @brief Logging subsystem interface.
 *
 * This module provides a logging framework with support for different log
 * levels, hierarchical loggers, and customizable output writers and formatters.
 */

#ifndef CFL_LOG_H_

#define CFL_LOG_H_

#include <stdarg.h>
#include <stdio.h>

#include "cfl_buffer.h"
#include "cfl_str.h"
#include "cfl_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Macro to declare module-specific logging functions */
#define DECLARE_LOGGER_FUN(level_name)                                         \
  extern void cfl_log_##level_name(CFL_LOGGERP logger, const char *message, ...)

/**
 * @brief Log levels enumeration.
 */
typedef enum {
  CFL_LOG_LEVEL_OFF = 0,   /**< Logging disabled */
  CFL_LOG_LEVEL_ERROR = 1, /**< Error conditions */
  CFL_LOG_LEVEL_WARN = 2,  /**< Warning conditions */
  CFL_LOG_LEVEL_INFO = 3,  /**< Informational messages */
  CFL_LOG_LEVEL_DEBUG = 4, /**< Debugging information */
  CFL_LOG_LEVEL_TRACE = 5  /**< Fine-grained trace information */
} CFL_LOG_LEVEL;

/** @brief Function pointer for writing log data */
typedef void (*CFL_LOG_WRITE)(void *handle, const char *data, CFL_UINT32 len);
/** @brief Function pointer for closing log handle */
typedef void (*CFL_LOG_CLOSE)(void *handle);
/** @brief Function pointer for formatting log messages */
typedef void (*CFL_LOG_FORMATTER)(CFL_STRP buffer, CFL_LOG_LEVEL level,
                                  const char *id, const char *filePathname,
                                  CFL_UINT32 line, const char *message,
                                  va_list varArgs);

/**
 * @brief Logger structure.
 */
typedef struct _CFL_LOGGER {
  CFL_LOG_LEVEL level;  /**< Current active log level for this logger */
  const char *id;       /**< Logger Identifier */
  const char *parentId; /**< Parent Logger Identifier */
  void *node;           /**< Internal node pointer */
} CFL_LOGGER, *CFL_LOGGERP;

/**
 * @brief Registers a logger with the system.
 * @param logger Pointer to the logger structure.
 */
extern void cfl_log_register(CFL_LOGGERP logger);

/**
 * @brief Gets the root logger.
 * @return Pointer to the root logger.
 */
extern CFL_LOGGERP cfl_log_root(void);

/**
 * @brief Finds a logger by pathname.
 * @param logger Base logger to search from.
 * @param loggerPathname Pathname of the child logger.
 * @return Pointer to the found logger, or NULL if not found.
 */
extern CFL_LOGGERP cfl_log_find(CFL_LOGGERP logger, const char *loggerPathname);

/**
 * @brief Finds a logger from the root.
 * @param loggerPathname Pathname of the logger.
 * @return Pointer to the found logger, or NULL if not found.
 */
extern CFL_LOGGERP cfl_log_findFromRoot(const char *loggerPathname);

/**
 * @brief Writes a log message using va_list.
 * @param logger Pointer to the logger.
 * @param level Log level of the message.
 * @param format Printf-style format string.
 * @param varArgs Variable argument list.
 */
extern void cfl_log_writeArgs(CFL_LOGGERP logger, CFL_LOG_LEVEL level,
                              const char *format, va_list varArgs);

/**
 * @brief Writes a log message.
 * @param logger Pointer to the logger.
 * @param level Log level of the message.
 * @param format Printf-style format string.
 * @param ... Variable arguments.
 */
extern void cfl_log_write(CFL_LOGGERP logger, CFL_LOG_LEVEL level,
                          const char *format, ...);

/**
 * @brief Writes a log message with file location info using va_list.
 * @param logger Pointer to the logger.
 * @param level Log level.
 * @param filePathname Source file name.
 * @param line Source line number.
 * @param message Format string.
 * @param varArgs Variable argument list.
 */
extern void cfl_log_writeArgsFL(CFL_LOGGERP logger, CFL_LOG_LEVEL level,
                                const char *filePathname, CFL_UINT32 line,
                                const char *message, va_list varArgs);

/**
 * @brief Writes a log message with file location info.
 * @param logger Pointer to the logger.
 * @param level Log level.
 * @param filePathname Source file name.
 * @param line Source line number.
 * @param message Format string.
 * @param ... Variable arguments.
 */
extern void cfl_log_writeFL(CFL_LOGGERP logger, CFL_LOG_LEVEL level,
                            const char *filePathname, CFL_UINT32 line,
                            const char *message, ...);

/**
 * @brief Sets a custom writer for the logger.
 * @param logger Pointer to the logger.
 * @param data User data provided to write/close functions.
 * @param write Function to write log data.
 * @param close Function to close the writer.
 */
extern void cfl_log_setWriter(CFL_LOGGERP logger, void *data,
                              CFL_LOG_WRITE write, CFL_LOG_CLOSE close);

/**
 * @brief Sets the logger to write to a file by path.
 * @param logger Pointer to the logger.
 * @param pathfilename Path to the log file.
 */
extern void cfl_log_setFile(CFL_LOGGERP logger, const char *pathfilename);

/**
 * @brief Sets the logger to write to an open file handle.
 * @param logger Pointer to the logger.
 * @param fileHandle Open file handle (FILE*).
 */
extern void cfl_log_setFileHandle(CFL_LOGGERP logger, FILE *fileHandle);

/**
 * @brief Gets the current log level of a logger.
 * @param logger Pointer to the logger.
 * @return Current log level.
 */
extern CFL_LOG_LEVEL cfl_log_level(CFL_LOGGERP logger);

/**
 * @brief Sets the log level for a logger.
 * @param logger Pointer to the logger.
 * @param newLevel New log level.
 */
extern void cfl_log_setLevel(CFL_LOGGERP logger, CFL_LOG_LEVEL newLevel);

/**
 * @brief Sets the log level by name string.
 * @param logger Pointer to the logger.
 * @param level Name of the level (e.g., "INFO", "DEBUG").
 */
extern void cfl_log_setLevelByName(CFL_LOGGERP logger, const char *level);

/**
 * @brief Sets the logger to use the default formatter.
 * @param logger Pointer to the logger.
 */
extern void cfl_log_setDefaultFormat(CFL_LOGGERP logger);

/**
 * @brief Sets the logger to use GELF (Graylog Extended Log Format).
 * @param logger Pointer to the logger.
 */
extern void cfl_log_setGelfFormat(CFL_LOGGERP logger);

/**
 * @brief Sets a custom formatter for the logger.
 * @param logger Pointer to the logger.
 * @param formatter Custom formatter function.
 */
extern void cfl_log_setFormatter(CFL_LOGGERP logger,
                                 CFL_LOG_FORMATTER formatter);

/**
 * @brief Internal debug helper.
 * @param filePathname Source file.
 * @param message Message format.
 * @param ... Arguments.
 */
extern void __cfl_log_debug(char *filePathname, char *message, ...);

DECLARE_LOGGER_FUN(error);
DECLARE_LOGGER_FUN(warn);
DECLARE_LOGGER_FUN(info);
DECLARE_LOGGER_FUN(debug);
DECLARE_LOGGER_FUN(trace);

/** @brief Defines a static logger variable */
#define CFL_LOGGER(var)                                                        \
  static struct _CFL_LOGGER var = {CFL_LOG_LEVEL_TRACE, __FILE__, "", NULL}
/** @brief Defines a static logger variable in a group */
#define CFL_LOGGER_GROUP(var, group)                                           \
  static struct _CFL_LOGGER var = {CFL_LOG_LEVEL_TRACE, __FILE__, group, NULL}
/** @brief Defines a static logger variable with specific ID */
#define CFL_LOGGER_ID(var, id)                                                 \
  static struct _CFL_LOGGER var = {CFL_LOG_LEVEL_TRACE, id, "", NULL}
/** @brief Defines a static logger variable with ID and group */
#define CFL_LOGGER_ID_GROUP(var, id, group)                                    \
  static struct _CFL_LOGGER var = {CFL_LOG_LEVEL_TRACE, id, group, NULL}

/** @brief Checks if a log level is active for the given logger */
#define CFL_LOG_ACTIVE(l, ll) ((l).level >= (ll))

#ifndef __BORLANDC__
/** @brief Logs an ERROR message if level matches, including file/line info */
#define CFL_LOG_ERROR(logger, ...)                                             \
  if (logger.level >= CFL_LOG_LEVEL_ERROR)                                     \
  cfl_log_writeFL(&logger, CFL_LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
/** @brief Logs a WARN message if level matches, including file/line info */
#define CFL_LOG_WARN(logger, ...)                                              \
  if (logger.level >= CFL_LOG_LEVEL_WARN)                                      \
  cfl_log_writeFL(&logger, CFL_LOG_LEVEL_WARN, __FILE__, __LINE__, __VA_ARGS__)
/** @brief Logs an INFO message if level matches, including file/line info */
#define CFL_LOG_INFO(logger, ...)                                              \
  if (logger.level >= CFL_LOG_LEVEL_INFO)                                      \
  cfl_log_writeFL(&logger, CFL_LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
/** @brief Logs a DEBUG message if level matches, including file/line info */
#define CFL_LOG_DEBUG(logger, ...)                                             \
  if (logger.level >= CFL_LOG_LEVEL_DEBUG)                                     \
  cfl_log_writeFL(&logger, CFL_LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
/** @brief Logs a TRACE message if level matches, including file/line info */
#define CFL_LOG_TRACE(logger, ...)                                             \
  if (logger.level >= CFL_LOG_LEVEL_TRACE)                                     \
  cfl_log_writeFL(&logger, CFL_LOG_LEVEL_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif