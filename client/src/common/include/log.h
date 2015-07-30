#ifndef INSTAMSG_LOGGER
#define INSTAMSG_LOGGER

#include "../../communication/include/fs.h"

typedef struct Logger Logger;
struct Logger
{
    FileSystem *medium;
};

#define INSTAMSG_LOG_LEVEL_DISABLED 0
#define INSTAMSG_LOG_LEVEL_INFO     1
#define INSTAMSG_LOG_LEVEL_ERROR    2
#define INSTAMSG_LOG_LEVEL_DEBUG    3


Logger* get_new_logger(void *arg);
void release_logger(Logger*);

void info_log(Logger *logger, char *fmt, ...);
void error_log(Logger *logger, char *fmt, ...);
void debug_log(Logger *logger, char *fmt, ...);

int currentLogLevel;

Logger *logger;

#endif
