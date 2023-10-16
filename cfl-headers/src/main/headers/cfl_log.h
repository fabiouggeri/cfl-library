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

enum _CFL_LOG_LEVEL {LOG_LEVEL_OFF = 0, LOG_LEVEL_ERROR, LOG_LEVEL_WARN, LOG_LEVEL_INFO, LOG_LEVEL_DEBUG, LOG_LEVEL_TRACE};

typedef void (* CFL_LOG_WRITE)(void *handle, char *data, CFL_UINT32 len);
typedef void (* CFL_LOG_CLOSE)(void *handle);
typedef void (* CFL_LOG_FORMATTER)(CFL_LOG_LEVEL level, CFL_STRP buffer, char *message, va_list varArgs);

struct _CFL_LOGGER {
   CFL_LOG_LEVEL level;
   char          *id;
   char          *parentId;
   void          *node;
};

extern void cfl_log_register(CFL_LOGGERP logger);
extern CFL_LOGGERP cfl_log_root(void);
extern CFL_LOGGERP cfl_log_find(CFL_LOGGERP logger, char *loggerPathname);
extern CFL_LOGGERP cfl_log_findFromRoot(char *loggerPathname);
extern void cfl_log_writeArgs(CFL_LOGGERP logger, CFL_LOG_LEVEL level,  char * format, va_list varArgs);
extern void cfl_log_write(CFL_LOGGERP logger, CFL_LOG_LEVEL level,  char * format, ...);
extern void cfl_log_setWriter(CFL_LOGGERP logger, void *data, CFL_LOG_WRITE write, CFL_LOG_CLOSE close);
extern void cfl_log_setFile(CFL_LOGGERP logger, const char *pathfilename);
extern void cfl_log_setFileHandle(CFL_LOGGERP logger, FILE *fileHandle);
extern CFL_LOG_LEVEL cfl_log_level(CFL_LOGGERP logger);
extern void cfl_log_setLevel(CFL_LOGGERP logger, CFL_LOG_LEVEL newLevel);
extern void cfl_log_setDefaultFormat(CFL_LOGGERP logger);
extern void cfl_log_setGelfFormat(CFL_LOGGERP logger);
extern void cfl_log_setFormatter(CFL_LOGGERP logger, CFL_LOG_FORMATTER formatter);


#define DEFINE_LOGGER_FUN(logger, logger_id, logger_root_id, level_name, level_value) \
   static inline void cfl_log_##level_name(char *message, ...) { \
      if (logger.level >= level_value) { \
         va_list varArgs; \
         va_start(varArgs, message); \
         cfl_log_writeArgs(&logger, level_value, message, varArgs); \
         va_end(varArgs); \
      } \
   }

#define CFL_LOGGER_INIT(v, y) \
   static CFL_LOGGER v = {LOG_LEVEL_TRACE, __FILE__, y, NULL}; \
   DEFINE_LOGGER_FUN(v, __FILE__, y, error, LOG_LEVEL_ERROR) \
   DEFINE_LOGGER_FUN(v, __FILE__, y, warn, LOG_LEVEL_WARN) \
   DEFINE_LOGGER_FUN(v, __FILE__, y, info, LOG_LEVEL_INFO) \
   DEFINE_LOGGER_FUN(v, __FILE__, y, debug, LOG_LEVEL_DEBUG) \
   DEFINE_LOGGER_FUN(v, __FILE__, y, trace, LOG_LEVEL_TRACE)

#define CFL_LOGGER_INIT_NAME(v, x, y) \
   static CFL_LOGGER v = {LOG_LEVEL_TRACE, x, y, NULL}; \
   DEFINE_LOGGER_FUN(v, x, y, error, LOG_LEVEL_ERROR) \
   DEFINE_LOGGER_FUN(v, x, y, warn, LOG_LEVEL_WARN) \
   DEFINE_LOGGER_FUN(v, x, y, info, LOG_LEVEL_INFO) \
   DEFINE_LOGGER_FUN(v, x, y, debug, LOG_LEVEL_DEBUG) \
   DEFINE_LOGGER_FUN(v, x, y, trace, LOG_LEVEL_TRACE)

#undef DEFINE_LOGGER_FUN

#if __STDC_VERSION__ >= 199901L
#define CFL_LOG_ERROR(logger, ...) if (logger.level >= LOG_LEVEL_ERROR) cfl_log_write(&logger, LOG_LEVEL_ERROR, __VA_ARGS__)
#define CFL_LOG_WARN(logger, ...) if (logger.level >= LOG_LEVEL_WARN) cfl_log_write(&logger, LOG_LEVEL_WARN, __VA_ARGS__)
#define CFL_LOG_INFO(logger, ...) if (logger.level >= LOG_LEVEL_INFO) cfl_log_write(&logger, LOG_LEVEL_INFO, __VA_ARGS__)
#define CFL_LOG_DEBUG(logger, ...) if (logger.level >= LOG_LEVEL_DEBUG) cfl_log_write(&logger, LOG_LEVEL_DEBUG, __VA_ARGS__)
#define CFL_LOG_TRACE(logger, ...) if (logger.level >= LOG_LEVEL_TRACE) cfl_log_write(&logger, LOG_LEVEL_TRACE, __VA_ARGS__)
#endif

#endif
