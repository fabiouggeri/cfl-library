/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#ifndef CFL_LOG_H_

#define CFL_LOG_H_

#include <stdarg.h>
#include <stdio.h>

#include "cfl_types.h"
#include "cfl_str.h"
#include "cfl_buffer.h"


#ifdef __cplusplus
extern "C" {
#endif

#define DECLARE_LOGGER_FUN(level_name) extern void cfl_log_##level_name(CFL_LOGGERP logger, const char *message, ...)

typedef enum { CFL_LOG_LEVEL_OFF   = 0, 
               CFL_LOG_LEVEL_ERROR = 1, 
               CFL_LOG_LEVEL_WARN  = 2,
               CFL_LOG_LEVEL_INFO  = 3,
               CFL_LOG_LEVEL_DEBUG = 4,
               CFL_LOG_LEVEL_TRACE = 5 } CFL_LOG_LEVEL;

typedef void (* CFL_LOG_WRITE)(void *handle, const char *data, CFL_UINT32 len);
typedef void (* CFL_LOG_CLOSE)(void *handle);
typedef void (* CFL_LOG_FORMATTER)(CFL_STRP buffer, CFL_LOG_LEVEL level, const char *id, const char *filePathname, CFL_UINT32 line,
                                   const char *message, va_list varArgs);

typedef struct _CFL_LOGGER {
   CFL_LOG_LEVEL level;
   const char    *id;
   const char    *parentId;
   void          *node;
} CFL_LOGGER, *CFL_LOGGERP;

extern void cfl_log_register(CFL_LOGGERP logger);
extern CFL_LOGGERP cfl_log_root(void);
extern CFL_LOGGERP cfl_log_find(CFL_LOGGERP logger, const char *loggerPathname);
extern CFL_LOGGERP cfl_log_findFromRoot(const char *loggerPathname);
extern void cfl_log_writeArgs(CFL_LOGGERP logger, CFL_LOG_LEVEL level, const char * format, va_list varArgs);
extern void cfl_log_write(CFL_LOGGERP logger, CFL_LOG_LEVEL level,  const char * format, ...);
extern void cfl_log_writeArgsFL(CFL_LOGGERP logger, CFL_LOG_LEVEL level, const char *filePathname, CFL_UINT32 line, const char *message, va_list varArgs);
extern void cfl_log_writeFL(CFL_LOGGERP logger, CFL_LOG_LEVEL level, const char *filePathname, CFL_UINT32 line, const char *message, ...);
extern void cfl_log_setWriter(CFL_LOGGERP logger, void *data, CFL_LOG_WRITE write, CFL_LOG_CLOSE close);
extern void cfl_log_setFile(CFL_LOGGERP logger, const char *pathfilename);
extern void cfl_log_setFileHandle(CFL_LOGGERP logger, FILE *fileHandle);
extern CFL_LOG_LEVEL cfl_log_level(CFL_LOGGERP logger);
extern void cfl_log_setLevel(CFL_LOGGERP logger, CFL_LOG_LEVEL newLevel);
extern void cfl_log_setLevelByName(CFL_LOGGERP logger, const char *level);
extern void cfl_log_setDefaultFormat(CFL_LOGGERP logger);
extern void cfl_log_setGelfFormat(CFL_LOGGERP logger);
extern void cfl_log_setFormatter(CFL_LOGGERP logger, CFL_LOG_FORMATTER formatter);
extern void __cfl_log_debug(char *filePathname, char *message, ...);

DECLARE_LOGGER_FUN(error);
DECLARE_LOGGER_FUN(warn);
DECLARE_LOGGER_FUN(info);
DECLARE_LOGGER_FUN(debug);
DECLARE_LOGGER_FUN(trace);

#define CFL_LOGGER(var)                     static struct _CFL_LOGGER var = {CFL_LOG_LEVEL_TRACE, __FILE__, "", NULL}
#define CFL_LOGGER_GROUP(var, group)        static struct _CFL_LOGGER var = {CFL_LOG_LEVEL_TRACE, __FILE__, group, NULL}
#define CFL_LOGGER_ID(var, id)              static struct _CFL_LOGGER var = {CFL_LOG_LEVEL_TRACE, id, "", NULL}
#define CFL_LOGGER_ID_GROUP(var, id, group) static struct _CFL_LOGGER var = {CFL_LOG_LEVEL_TRACE, id, group, NULL}

#define CFL_LOG_ACTIVE(l, ll) ((l).level >= (ll))

#ifndef __BORLANDC__
   #define CFL_LOG_ERROR(logger, ...) if (logger.level >= CFL_LOG_LEVEL_ERROR) \
                                                 cfl_log_writeFL(&logger, CFL_LOG_LEVEL_ERROR, __FILE__, __LINE__, __VA_ARGS__)
   #define CFL_LOG_WARN(logger, ...)  if (logger.level >= CFL_LOG_LEVEL_WARN) \
                                                 cfl_log_writeFL(&logger, CFL_LOG_LEVEL_WARN, __FILE__, __LINE__, __VA_ARGS__)
   #define CFL_LOG_INFO(logger, ...)  if (logger.level >= CFL_LOG_LEVEL_INFO) \
                                                 cfl_log_writeFL(&logger, CFL_LOG_LEVEL_INFO, __FILE__, __LINE__, __VA_ARGS__)
   #define CFL_LOG_DEBUG(logger, ...) if (logger.level >= CFL_LOG_LEVEL_DEBUG) \
                                                 cfl_log_writeFL(&logger, CFL_LOG_LEVEL_DEBUG, __FILE__, __LINE__, __VA_ARGS__)
   #define CFL_LOG_TRACE(logger, ...) if (logger.level >= CFL_LOG_LEVEL_TRACE) \
                                                 cfl_log_writeFL(&logger, CFL_LOG_LEVEL_TRACE, __FILE__, __LINE__, __VA_ARGS__)
#endif

#ifdef __cplusplus
}
#endif

#endif