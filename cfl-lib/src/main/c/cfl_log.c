#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "cfl_log.h"
#include "cfl_str.h"
#include "cfl_thread.h"
#include "cfl_socket.h"
#include "cfl_atomic.h"
#include "cfl_list.h"

#if defined(CFL_OS_LINUX)
   #define _strnicmp(s1, s2, n) strncasecmp(s1, s2, n)
#elif defined( __BORLANDC__ )
   #define _strnicmp strnicmp
#endif

#define LOCK_INIT_LOGGER(var)   while (! cfl_atomic_compareAndSetBoolean(&(var), CFL_FALSE, CFL_TRUE)) cfl_thread_yield()
#define UNLOCK_INIT_LOGGER(var) cfl_atomic_setBoolean(&(var), CFL_FALSE)

#define IS_ROOT_NODE(node) ((node)->parent == NULL)

#define INIT_ROOT_NODE() if (s_rootNode.logger == NULL) initRootNode()

#define DEFINE_LOGGER_FUN(level_name, level_value) \
   void cfl_log_##level_name(CFL_LOGGERP logger, const char *message, ...) { \
      if (logger->level >= level_value) { \
         va_list varArgs; \
         va_start(varArgs, message); \
         cfl_log_writeArgs(logger, level_value, message, varArgs); \
         va_end(varArgs); \
      } \
   }

typedef struct {
   void           *data;
   CFL_LOG_WRITE  write;
   CFL_LOG_CLOSE  close;
   CFL_UINT32     refCount;
} CFL_LOG_WRITER, *CFL_LOG_WRITERP;


typedef struct _CFL_LOGGER_NODE {
   CFL_STR                 id;
   CFL_LIST                children;
   struct _CFL_LOGGER_NODE *parent;
   CFL_LOGGERP             logger;
   CFL_LOG_WRITERP         writer;
   CFL_LOG_FORMATTER       format;
} CFL_LOGGER_NODE, *CFL_LOGGER_NODEP;

static void initRootNode(void);
static void default_log_formatter(CFL_STRP buffer, CFL_LOG_LEVEL level, const char *id, const char *filePathname, CFL_UINT32 line,
                                  const char *message, va_list varArgs);
static void default_log_writer(void *handle, const char *data, CFL_UINT32 len);
static void default_log_close(void *handle);

static char *level_name[] = {"OFF", "ERROR", "WARN", "INFO", "DEBUG", "TRACE"};

static CFL_BOOL s_locked = CFL_FALSE;

static CFL_LOGGER_NODE s_rootNode = { 0 };

static char *s_missingId = "__missing__";

static struct _CFL_LOGGER s_rootLogger = {CFL_LOG_LEVEL_ERROR, "", "", &s_rootNode};

static char s_hostName[256] = {0};

static char * sub_path(const char *filePathname, CFL_UINT8 maxPathSep) {
   char *path = (char *) filePathname;
   CFL_UINT8 count = 0;
   while (*path != '\0') {
      path++;
   }
   while (path != filePathname) {
      char c = *path;
      if (c == '/' || c == '\\') {
         count++;
         if (count >= maxPathSep) {
            return path;
         }
      }
      path--;
   }
   return path;
}

static char *file_name(const char *pathFilename) {
   char *index = (char *) pathFilename;
   char *filename = NULL;
   char c = *index;
   while (c != '\0') {
      if (c == '/' || c == '\\') {
         filename = index + 1;
      }
      index++;
      c = *index;
   }
   return filename != NULL ? filename : (char *) pathFilename;
}

static CFL_LOGGER_NODEP logger_node(CFL_LOGGERP logger) {
   if (logger->node == NULL) {
      cfl_log_register(logger);
   }
   return (CFL_LOGGER_NODEP) logger->node;
}

static void set_log_writer(CFL_LOGGER_NODEP node, CFL_LOG_WRITERP logWriter) {
   if (node->writer != NULL) {
      node->writer->refCount--;
      if (node->writer->refCount == 0) {
         if (node->writer->close != NULL) {
            node->writer->close(node->writer->data);
         }
         CFL_MEM_FREE(node->writer);
      }
   }
   if (logWriter != NULL) {
      logWriter->refCount++;
   }
   node->writer = logWriter;
}

static void node_setWriter(CFL_LOGGER_NODEP node, CFL_LOG_WRITERP logWriter) {
   CFL_UINT32 i;
   CFL_UINT32 len;
   INIT_ROOT_NODE();
   set_log_writer(node, logWriter);
   len = cfl_list_length(&node->children);
   for (i = 0; i < len; i++) {
      node_setWriter((CFL_LOGGER_NODEP) cfl_list_get(&node->children, i), logWriter);
   }
}

static void node_setFormatter(CFL_LOGGER_NODEP node, CFL_LOG_FORMATTER formatter) {
   CFL_UINT32 i;
   CFL_UINT32 len;
   INIT_ROOT_NODE();
   node->format = formatter;
   len = cfl_list_length(&node->children);
   for (i = 0; i < len; i++) {
      node_setFormatter((CFL_LOGGER_NODEP) cfl_list_get(&node->children, i), formatter);
   }
}


static void node_setLevel(CFL_LOGGER_NODEP node, CFL_LOG_LEVEL level) {
   CFL_UINT32 i;
   CFL_UINT32 len;
   INIT_ROOT_NODE();
   node->logger->level = level;
   len = cfl_list_length(&node->children);
   for (i = 0; i < len; i++) {
      node_setLevel((CFL_LOGGER_NODEP) cfl_list_get(&node->children, i), level);
   }
}

static CFL_LOG_WRITERP newLoggerWriter(void *data, CFL_LOG_WRITE write, CFL_LOG_CLOSE close) {
   CFL_LOG_WRITERP logWriter = (CFL_LOG_WRITERP) CFL_MEM_ALLOC(sizeof(CFL_LOG_WRITER));
   memset(logWriter, 0, sizeof(CFL_LOG_WRITER));
   logWriter->data = data;
   logWriter->write = write;
   logWriter->close = close;
   logWriter->refCount = 0;
   return logWriter;
}

static void initRootNode(void) {
   CFL_LOG_WRITERP logWriter;
   if (s_rootNode.logger != NULL) {
      return;
   }
   logWriter = (CFL_LOG_WRITERP) CFL_MEM_ALLOC(sizeof(CFL_LOG_WRITER));
   memset(logWriter, 0, sizeof(CFL_LOG_WRITER));
   logWriter->data = stderr;
   logWriter->write = default_log_writer;
   logWriter->close = default_log_close;
   logWriter->refCount = 1;
   cfl_str_initConst(&s_rootNode.id, "/");
   cfl_list_init(&s_rootNode.children, 2);
   s_rootNode.parent = NULL;
   s_rootNode.logger = &s_rootLogger;
   s_rootNode.format = default_log_formatter;
   set_log_writer(&s_rootNode, logWriter);
}

static CFL_LOGGER_NODEP addNode(CFL_LOGGER_NODEP parent, CFL_LOGGERP logger, CFL_STRP id) {
   CFL_LOGGER_NODEP node = (CFL_LOGGER_NODEP) CFL_MEM_ALLOC(sizeof(CFL_LOGGER_NODE));
   memset(node, 0, sizeof(CFL_LOGGER_NODE));
   cfl_list_add(&parent->children, node);
   cfl_str_initValue(&node->id, cfl_str_getPtr(id));
   cfl_list_init(&node->children, 0);
   node->parent = parent;
   node->logger = logger;
   node->format = parent->format;
   set_log_writer(node, parent->writer);
   logger->level = parent->logger->level;
   logger->node = node;
   return node;
}

static CFL_UINT32 nextNodeId(CFL_STRP nodeId, CFL_STRP pathId, CFL_UINT32 start) {
   CFL_UINT32 i;
   CFL_UINT32 len = cfl_str_length(pathId);

   cfl_str_clear(nodeId);
   for (i = start; i < len; i++) {
      char c = cfl_str_charAt(pathId, i);
      if (c == '/' || c == '\\') {
         break;
      }
      cfl_str_appendChar(nodeId, c);   
   }
   return i + 1;
}

static CFL_LOGGER_NODEP findNodePath(CFL_LOGGER_NODEP rootNode, CFL_STRP searchPath, CFL_BOOL createMissing) {
   CFL_LOGGER_NODEP parent;
   CFL_UINT32 len;
   CFL_UINT32 i;
   CFL_UINT32 start;
   CFL_STR nodeId;

   if (searchPath == NULL                    ||
       cfl_str_isBlank(searchPath)           || 
       cfl_str_bufferEquals(searchPath, "/") || 
       cfl_str_bufferEquals(searchPath, "\\")) {
      return rootNode;
   }
   cfl_str_init(&nodeId);
   parent = rootNode;
   start = cfl_str_bufferStartsWith(searchPath, "/") || cfl_str_bufferStartsWith(searchPath, "\\") ? 1 : 0;
   start = nextNodeId(&nodeId, searchPath, start);
   while (parent != NULL && ! cfl_str_isEmpty(&nodeId)) {
      CFL_LOGGER_NODEP nodeFound = NULL;
      len = cfl_list_length(&parent->children);
      for (i = 0; i < len; i++) {
         CFL_LOGGER_NODEP node = (CFL_LOGGER_NODEP) cfl_list_get(&parent->children, i);
         if (cfl_str_equals(&node->id, &nodeId)) {
            start = nextNodeId(&nodeId, searchPath, start);
            nodeFound = node;
            break;
         }
      }
      if (nodeFound == NULL && createMissing) {
         CFL_LOGGERP logger = CFL_MEM_ALLOC(sizeof(struct _CFL_LOGGER));
         logger->id = s_missingId;
         logger->parentId = s_missingId;
         nodeFound = addNode(parent, logger, &nodeId);
         start = nextNodeId(&nodeId, searchPath, start);
      }
      parent = nodeFound;
   }
   cfl_str_free(&nodeId);
   return parent;
}

static char *hostName(void) {
   if (s_hostName[0] == '\0') {
      cfl_socket_hostname(s_hostName, sizeof(s_hostName));
   }
   return s_hostName;
}

static void default_log_close(void *handle) {
   if (handle != NULL) {
      fclose((FILE *)handle);
   }
}

static void default_log_writer(void *handle, const char *data, CFL_UINT32 len) {
   if (handle != NULL) {
      fwrite(data, sizeof(char), len, (FILE *)handle);
      fflush((FILE *)handle);
   }
}

static void default_log_formatter(CFL_STRP buffer, CFL_LOG_LEVEL level, const char *id, const char *filePathname, CFL_UINT32 line,
                                  const char *message, va_list varArgs) {
   time_t curTime;
   struct tm *tm;
   time(&curTime);
   tm = localtime(&curTime);

   cfl_str_setFormat(buffer, "%04d-%02d-%02dT%02d:%02d:%02d %-5s: ",
                     1900 + tm->tm_year, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, 
                     level_name[level]);
   if (id != NULL && id[0] != '\0') {
      cfl_str_appendFormat(buffer, "[%s] ", id);
   }
   if (level >= CFL_LOG_LEVEL_DEBUG && filePathname != NULL && filePathname[0] != '\0') {
      char *subPathname = sub_path(filePathname, 5);
      cfl_str_appendFormat(buffer, (subPathname != filePathname) ? "|...%s:%u| " : "|%s:%u| ", subPathname, line);
   }
   cfl_str_appendFormatArgs(buffer, message, varArgs);
   cfl_str_appendChar(buffer, '\n');
}

static int gelf_log_level(CFL_LOG_LEVEL level) {
   switch (level) {
      case CFL_LOG_LEVEL_ERROR:
         return 3;
      case CFL_LOG_LEVEL_WARN:
         return 4;
      case CFL_LOG_LEVEL_INFO:
         return 6;
      case CFL_LOG_LEVEL_DEBUG:
      case CFL_LOG_LEVEL_TRACE:
         return 7;
      default:
         return 0;
   }
}

static void gelf_log_formatter(CFL_STRP buffer, CFL_LOG_LEVEL level, const char *id, const char *filePathname, CFL_UINT32 line,
                               const char *message, va_list varArgs) {
   if (level >= CFL_LOG_LEVEL_DEBUG && filePathname != NULL && filePathname[0] != '\0') {
      char *subPathname = sub_path(filePathname, 5);
      if (subPathname != filePathname) {
         cfl_str_setFormat(buffer, "{\"version\":\"1.1\","
                                    "\"timestamp\":%ld,"
                                    "\"host\":\"%s\","
                                    "\"_log_id\":\"%s\","
                                    "\"_source\":\"...%s\","
                                    "\"_line\":\"%s\","
                                    "\"short_message\":\"",
                 time(NULL), hostName(), id, subPathname, line);
      } else {
         cfl_str_setFormat(buffer, "{\"version\":\"1.1\","
                                    "\"timestamp\":%ld,"
                                    "\"host\":\"%s\","
                                    "\"_log_id\":\"%s\","
                                    "\"_source\":\"%s\","
                                    "\"_line\":\"%s\","
                                    "\"short_message\":\"",
                 time(NULL), hostName(), id != NULL && id[0] != '\0' ? id : "ROOT", subPathname, line);
      }
   } else {
      cfl_str_setFormat(buffer, "{\"version\":\"1.1\","
                                 "\"timestamp\":%ld,"
                                 "\"host\":\"%s\","
                                 "\"_log_id\":\"%s\","
                                 "\"short_message\":\"",
              time(NULL), hostName(), id != NULL && id[0] != '\0' ? id : "ROOT");
   }
   cfl_str_appendFormatArgs(buffer, message, varArgs);
   cfl_str_appendFormat(buffer, "\",\"level\":%d}\n", gelf_log_level(level));
}

void cfl_log_register(CFL_LOGGERP logger) {
   CFL_LOGGER_NODEP parentNode;
   CFL_STR strId;
   CFL_STR strParent;

   cfl_str_initConst(&strId, file_name(logger->id));
   cfl_str_initConst(&strParent, logger->parentId);

   LOCK_INIT_LOGGER(s_locked);

   INIT_ROOT_NODE();

   parentNode = findNodePath(&s_rootNode, &strParent, CFL_TRUE);
   if (parentNode != NULL) {
      CFL_LOGGER_NODEP node = findNodePath(parentNode, &strId, CFL_FALSE);
      if (node == NULL) {
         addNode(parentNode, logger, &strId);
      } else {
         logger->level = node->logger->level;
         logger->node = node;
         if (node->logger->id == s_missingId) {
            CFL_MEM_FREE(node->logger);
         }
         node->logger = logger;
      }
   }
   UNLOCK_INIT_LOGGER(s_locked);

   cfl_str_free(&strId);
   cfl_str_free(&strParent);
}

CFL_LOGGERP cfl_log_root(void) {
   if (s_rootNode.logger == NULL) {
      LOCK_INIT_LOGGER(s_locked);
      initRootNode();
      UNLOCK_INIT_LOGGER(s_locked);
   }
   return &s_rootLogger;
}

CFL_LOGGERP cfl_log_find(CFL_LOGGERP logger, const char *loggerPathname) {
   CFL_LOGGER_NODEP node;
   CFL_STR path;

   cfl_str_initConst(&path, loggerPathname);
   LOCK_INIT_LOGGER(s_locked);
   INIT_ROOT_NODE();
   node = findNodePath(logger_node(logger), &path, CFL_TRUE);
   UNLOCK_INIT_LOGGER(s_locked);
   cfl_str_free(&path);
   return node != NULL ? node->logger : NULL;
}

CFL_LOGGERP cfl_log_findFromRoot(const char *loggerPathname) {
   return cfl_log_find(&s_rootLogger, loggerPathname);
}

void cfl_log_writeArgs(CFL_LOGGERP logger, CFL_LOG_LEVEL level, const char *message, va_list varArgs) {
   CFL_LOGGER_NODEP node = (CFL_LOGGER_NODEP) logger->node;
   if (node == NULL) {
      LOCK_INIT_LOGGER(s_locked);
      node = logger_node(logger);
      UNLOCK_INIT_LOGGER(s_locked);
   }
   if (logger->level >= level) {
      CFL_STR buffer;
      cfl_str_init(&buffer);
      node->format(&buffer, level, logger->parentId, NULL, 0, message, varArgs);
      node->writer->write(node->writer->data, cfl_str_getPtr(&buffer), cfl_str_length(&buffer));
      cfl_str_free(&buffer);
   }
}

void cfl_log_write(CFL_LOGGERP logger, CFL_LOG_LEVEL level, const char *message, ...) {
   va_list varArgs;
   va_start(varArgs, message);
   cfl_log_writeArgs(logger, level, message, varArgs);
   va_end(varArgs);
}

void cfl_log_writeArgsFL(CFL_LOGGERP logger, CFL_LOG_LEVEL level, const char *filePathname, CFL_UINT32 line, const char *message, va_list varArgs) {
   CFL_LOGGER_NODEP node = (CFL_LOGGER_NODEP) logger->node;
   if (node == NULL) {
      LOCK_INIT_LOGGER(s_locked);
      node = logger_node(logger);
      UNLOCK_INIT_LOGGER(s_locked);
   }
   if (logger->level >= level) {
      CFL_STR buffer;
      cfl_str_init(&buffer);
      node->format(&buffer, level, logger->parentId, filePathname, line, message, varArgs);
      node->writer->write(node->writer->data, cfl_str_getPtr(&buffer), cfl_str_length(&buffer));
      cfl_str_free(&buffer);
   }
}

void cfl_log_writeFL(CFL_LOGGERP logger, CFL_LOG_LEVEL level, const char *filePathname, CFL_UINT32 line, const char *message, ...) {
   va_list varArgs;
   va_start(varArgs, message);
   cfl_log_writeArgsFL(logger, level, filePathname, line, message, varArgs);
   va_end(varArgs);
}

void cfl_log_setWriter(CFL_LOGGERP logger, void *data, CFL_LOG_WRITE write, CFL_LOG_CLOSE close) {
   LOCK_INIT_LOGGER(s_locked);
   node_setWriter(logger_node(logger), newLoggerWriter(data, write, close));
   UNLOCK_INIT_LOGGER(s_locked);
}

void cfl_log_setFileHandle(CFL_LOGGERP logger, FILE *fileHandle) {
   LOCK_INIT_LOGGER(s_locked);
   node_setWriter(logger_node(logger), newLoggerWriter(fileHandle, default_log_writer, NULL));
   UNLOCK_INIT_LOGGER(s_locked);
}

void cfl_log_setFile(CFL_LOGGERP logger, const char *pathfilename) {
   LOCK_INIT_LOGGER(s_locked);
   if (_strnicmp(pathfilename, "stdout", 6) == 0) {
      node_setWriter(logger_node(logger), newLoggerWriter(stdout, default_log_writer, NULL));
   } else if (_strnicmp(pathfilename, "stderr", 6) == 0) {
      node_setWriter(logger_node(logger), newLoggerWriter(stderr, default_log_writer, NULL));
   } else {
      FILE *handle = fopen(pathfilename, "a");
      if (handle) {
         node_setWriter(logger_node(logger), newLoggerWriter(handle, default_log_writer, default_log_close));
      }
   }
   UNLOCK_INIT_LOGGER(s_locked);
}

CFL_LOG_LEVEL cfl_log_level(CFL_LOGGERP logger) {
   return logger->level;
}

void cfl_log_setLevel(CFL_LOGGERP logger, CFL_LOG_LEVEL newLevel) {
   LOCK_INIT_LOGGER(s_locked);
   node_setLevel(logger_node(logger), newLevel);
   UNLOCK_INIT_LOGGER(s_locked);
}

static CFL_LOG_LEVEL levelFromName(const char *level) {
   if (_strnicmp(level, "off", 3) == 0) {
      return CFL_LOG_LEVEL_OFF;
   } else if (_strnicmp(level, "warn", 4) == 0) {
      return CFL_LOG_LEVEL_WARN;
   } else if (_strnicmp(level, "info", 4) == 0) {
      return CFL_LOG_LEVEL_INFO;
   } else if (_strnicmp(level, "debug", 5) == 0) {
      return CFL_LOG_LEVEL_DEBUG;
   } else if (_strnicmp(level, "trace", 5) == 0) {
      return CFL_LOG_LEVEL_TRACE;
   }
   return CFL_LOG_LEVEL_ERROR;
}

void cfl_log_setLevelByName(CFL_LOGGERP logger, const char *level) {
   CFL_LOG_LEVEL newLevel = levelFromName(level);
   LOCK_INIT_LOGGER(s_locked);
   node_setLevel(logger_node(logger), newLevel);
   UNLOCK_INIT_LOGGER(s_locked);
}

void cfl_log_setDefaultFormat(CFL_LOGGERP logger) {
   CFL_LOGGER_NODEP node;
   LOCK_INIT_LOGGER(s_locked);
   node = logger_node(logger);
   node_setFormatter(node, default_log_formatter);
   UNLOCK_INIT_LOGGER(s_locked);
}

void cfl_log_setGelfFormat(CFL_LOGGERP logger) {
   CFL_LOGGER_NODEP node; 
   LOCK_INIT_LOGGER(s_locked);
   node = logger_node(logger);
   node_setFormatter(node, gelf_log_formatter);
   UNLOCK_INIT_LOGGER(s_locked);
}

void cfl_log_setFormatter(CFL_LOGGERP logger, CFL_LOG_FORMATTER formatter) {
   CFL_LOGGER_NODEP node;
   LOCK_INIT_LOGGER(s_locked);
   node = logger_node(logger);
   node_setFormatter(node, formatter);
   UNLOCK_INIT_LOGGER(s_locked);
}

void __cfl_log_debug(char *filePathname, char *message, ...) {
   va_list args;
   FILE *v = fopen(filePathname, "a"); 
   va_start(args, message);
   vfprintf(v, message, args); 
   va_end(args);
   fclose(v); 
}

DEFINE_LOGGER_FUN(error, CFL_LOG_LEVEL_ERROR)
DEFINE_LOGGER_FUN(warn , CFL_LOG_LEVEL_WARN)
DEFINE_LOGGER_FUN(info , CFL_LOG_LEVEL_INFO)
DEFINE_LOGGER_FUN(debug, CFL_LOG_LEVEL_DEBUG)
DEFINE_LOGGER_FUN(trace, CFL_LOG_LEVEL_TRACE)
