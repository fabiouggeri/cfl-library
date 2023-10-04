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

enum _CFL_LOG_LEVEL {LOG_LEVEL_UNINITIALIZED = 0, LOG_LEVEL_OFF, LOG_LEVEL_ERROR, LOG_LEVEL_WARN, LOG_LEVEL_INFO, LOG_LEVEL_DEBUG, LOG_LEVEL_TRACE};

typedef void (* CFL_LOG_WRITE)(CFL_LOGGERP logger, CFL_STRP message);
typedef void (* CFL_LOG_CLOSE)(CFL_LOGGERP logger);
typedef void (* CFL_LOG_FORMAT)(CFL_LOGGERP logger, CFL_STRP buffer, char *message, va_list varArgs);

struct _CFL_LOGGER {
   CFL_LOG_LEVEL  level;
   CFL_LOG_WRITE  write;
   CFL_LOG_CLOSE  close;
   struct _CFL_LOG_WRITER *logWriter;
   CFL_LOG_FORMAT format;
   void           *handle;
};

extern CFL_LOGGERP cfl_log_newFile(const char *pathfilename);
extern CFL_LOGGERP cfl_log_newFileHandle(FILE *fileHandle);
extern void cfl_log_free(CFL_LOGGERP logger);
extern void cfl_log_write(CFL_LOGGERP logger, CFL_LOG_LEVEL level,  char * format, va_list varArgs);
extern CFL_LOG_LEVEL cfl_log_level(CFL_LOGGERP logger);
extern void cfl_log_setLevel(CFL_LOGGERP logger, CFL_LOG_LEVEL newLevel);
extern void cfl_log_setDefaultFormat(CFL_LOGGERP logger);
extern void cfl_log_setGelfFormat(CFL_LOGGERP logger);
extern void cfl_log_setFormatter(CFL_LOGGERP logger, CFL_LOG_FORMAT formatter);


#define DEFINE_LOGGER_FUN(logger, name, level) \
   static inline void cfl_log_##name(char *message, ...) { \
      if ((logger)->level >= level) { \
         cfl_log_write(logger, level, message, args); \
      } \
   }

#define DEFINE_LATE_LOGGER_INIT_FUN(logger, logger_init, name, level) \
   static inline void cfl_log_##name(char *message, ...) { \
      if (logger->level >= level) { \
         cfl_log_write(logger, level, message, args); \
      } else if (logger->level == LOG_LEVEL_UNINITIALIZED) { \
         logger = logger_init; \
         if (logger->level >= level) { \
            cfl_log_write(logger, level, message, args); \
         }\
      } \
   }

#define CFL_LOGGER_INIT(x, y) \
   static CFL_LOGGER empty_##x = {0}; \
   static CFL_LOGGERP x = &empty_##x; \
   DEFINE_LATE_LOGGER_INIT_FUN(x, y, error, LOG_LEVEL_ERROR) \
   DEFINE_LATE_LOGGER_INIT_FUN(x, y, warn, LOG_LEVEL_WARN) \
   DEFINE_LATE_LOGGER_INIT_FUN(x, y, info, LOG_LEVEL_INFO) \
   DEFINE_LATE_LOGGER_INIT_FUN(x, y, debug, LOG_LEVEL_DEBUG) \
   DEFINE_LATE_LOGGER_INIT_FUN(x, y, trace, LOG_LEVEL_TRACE)

#define CFL_LOGGER_REF(x, y) \
   static CFL_LOGGERP x = y; \
   DEFINE_LOGGER_FUN(x, error, LOG_LEVEL_ERROR) \
   DEFINE_LOGGER_FUN(x, warn, LOG_LEVEL_WARN) \
   DEFINE_LOGGER_FUN(x, info, LOG_LEVEL_INFO) \
   DEFINE_LOGGER_FUN(x, debug, LOG_LEVEL_DEBUG) \
   DEFINE_LOGGER_FUN(x, trace, LOG_LEVEL_TRACE)

#undef DEFINE_LATE_LOGGER_INIT_FUN
#undef DEFINE_LOGGER_FUN

#endif
