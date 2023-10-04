#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "cfl_log.h"
#include "cfl_str.h"
#include "cfl_thread.h"

static char *level_name[] = {"UNINITIALIZED", "OFF", "ERROR", "WARN", "INFO", "DEBUG", "TRACE"};

static char s_hostName[256] = {0};

static char *hostName() {
   if (s_hostName[0] == '\0') {
#if defined(_WIN32)
   WSADATA wsadata;
   WSAStartup(MAKEWORD(1, 1), &wsadata);
#endif
   if (gethostname(s_hostName, sizeof(s_hostName)) != 0) {
      strcpy(s_hostName, "unknown");
   }
#if defined(_WIN32)
   WSACleanup();
#endif
   }
   return s_hostName;
}

static void default_log_close(CFL_LOGGERP logger) {
   FILE *handle = (FILE *) logger->handle;
   if (handle != NULL) {
      fclose(handle);
   }
}

static void default_log_writer(CFL_LOGGERP logger, CFL_STRP message) {
   FILE *handle = (FILE *) logger->handle;
   if (handle != NULL) {
      fwrite(cfl_str_getPtr(message), sizeof(char), cfl_str_length(message), handle);
      fflush(handle);
   }
}

static void default_log_formatter(CFL_LOGGERP logger, CFL_STRP buffer, char *message, va_list varArgs) {
   time_t curTime;
   struct tm *tm;
   time(&curTime);
   tm = localtime(&curTime);
   if (logger->level >= LOG_LEVEL_DEBUG) {
      cfl_str_setFormat(buffer, "%04d-%02d-%02dT%02d:%02d:%02d - %s: ",
                           1900 + tm->tm_year, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, level_name[logger->level]);
   } else {
      cfl_str_setFormat(buffer, "%04d-%02d-%02dT%02d:%02d:%02d - %s: ",
                           1900 + tm->tm_year, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, level_name[logger->level]);
   }
   cfl_str_appendFormatArgs(buffer, message, varArgs);
   cfl_str_appendChar(buffer, '\n');
}

static int gelf_log_level(CFL_LOG_LEVEL level) {
   switch (level) {
      case LOG_LEVEL_ERROR:
         return 3;
      case LOG_LEVEL_WARN:
         return 4;
      case LOG_LEVEL_INFO:
         return 6;
      case LOG_LEVEL_DEBUG:
      case LOG_LEVEL_TRACE:
         return 7;
      default:
         return 0;
   }
}


static void gelf_log_formatter(CFL_LOGGERP logger, CFL_STRP buffer, char *message, va_list varArgs) {
   time_t curTime;
   struct tm *tm;
   time(&curTime);
   tm = localtime(&curTime);
   cfl_str_setFormat(buffer, "{\"version\":\"1.1\","
                              "\"timestamp\":%ld,"
                              "\"host\":\"%s\","
                              "\"short_message\":\"",
           time(NULL), hostName());
   cfl_str_appendFormatArgs(buffer, message, varArgs);
   cfl_str_appendFormat(buffer, ",\"level\":%d}\n", gelf_log_level(logger->level));
}

CFL_LOGGERP cfl_log_newFile(const char *pathfilename) {
   CFL_LOGGERP logger;
   if (stricmp(pathfilename, "stdout") == 0) {
      return cfl_log_newFileHandle(stdout);
   }
   logger = (CFL_LOGGERP) malloc(sizeof(struct _CFL_LOGGER));
   if (logger == NULL) {
      return NULL;
   }
   logger->level = LOG_LEVEL_ERROR;
   logger->format = default_log_formatter;
   logger->handle = fopen(pathfilename, "a");
   logger->write = default_log_writer;
   logger->close = default_log_close;
   return logger;
}

CFL_LOGGERP cfl_log_newFileHandle(FILE* fileHandle) {
   CFL_LOGGERP logger = (CFL_LOGGERP) malloc(sizeof(struct _CFL_LOGGER));
   if (logger == NULL) {
      return NULL;
   }
   logger->handle = fileHandle;
   logger->write = default_log_writer;
   logger->close = NULL;
   return logger;
}

void cfl_log_free(CFL_LOGGERP logger) {
   if (logger == NULL) {
      return;
   }
   if (logger->close != NULL) {
      logger->close(logger);
   }
   free(logger);
}

void cfl_log_write(CFL_LOGGERP logger, CFL_LOG_LEVEL level, char *message, va_list varArgs) {
   CFL_UINT32 i;
   CFL_STR buffer;
   cfl_str_init(&buffer);
   logger->format(logger, &buffer, message, varArgs);
   logger->write(logger, &buffer);
   cfl_str_free(&buffer);
}

CFL_LOG_LEVEL cfl_log_level(CFL_LOGGERP logger) {
   return logger->level;
}

void cfl_log_setLevel(CFL_LOGGERP logger, CFL_LOG_LEVEL newLevel) {
   logger->level = newLevel;
}

void cfl_log_setDefaultFormat(CFL_LOGGERP logger) {
   if (logger != NULL) {
      logger->format = default_log_formatter;
   }
}

void cfl_log_setGelfFormat(CFL_LOGGERP logger) {
   if (logger != NULL) {
      logger->format = gelf_log_formatter;
   }
}

void cfl_log_setFormatter(CFL_LOGGERP logger, CFL_LOG_FORMAT formatter) {
   if (logger != NULL) {
      logger->format = formatter;
   }
}
