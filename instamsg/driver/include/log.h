#ifndef INSTAMSG_LOGGER
#define INSTAMSG_LOGGER

#include "./serial_logger.h"
#include "device_file_system.h"

#ifdef FILE_SYSTEM_INTERFACE_ENABLED
typedef struct FileLogger FileLogger;
struct FileLogger
{
    FileSystem fs;
};

void init_file_logger(FileLogger *fileLogger, void *arg);
void release_file_logger(FileLogger *fileLogger);

FileLogger fileLogger;
#endif


#define INSTAMSG_LOG_LEVEL_DISABLED 0
#define INSTAMSG_LOG_LEVEL_INFO     1
#define INSTAMSG_LOG_LEVEL_ERROR    2
#define INSTAMSG_LOG_LEVEL_DEBUG    3

typedef int (*LOG_WRITE_FUNC)(void *logger_medium, unsigned char *buffer, int len);

void info_log(char *fmt, ...);
void error_log(char *fmt, ...);
void debug_log(char *fmt, ...);

int currentLogLevel;


#endif
