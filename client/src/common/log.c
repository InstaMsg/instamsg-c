/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

#include "./include/log.h"

#define MAX_LENGTH_LOG_ALLOWED 1000

#define LOG_COMMON_CODE(level)                                                                      \
    if(level > currentLogLevel)                                                                     \
        return;                                                                                     \
                                                                                                    \
    unsigned char formatted_string[MAX_LENGTH_LOG_ALLOWED];                                         \
    va_list argptr;                                                                                 \
                                                                                                    \
    va_start(argptr, fmt);                                                                          \
    vsnprintf(formatted_string, MAX_LENGTH_LOG_ALLOWED, fmt, argptr);                               \
    va_end(argptr);                                                                                 \
                                                                                                    \
    logger->medium->write(logger->medium, formatted_string, strlen(formatted_string));


Logger* get_new_logger(void *arg)
{
    Logger *logger = (Logger*)malloc(sizeof(Logger));

    // Here, physical medium is a file-system.
	logger->medium = get_new_file_system(arg);

    return logger;
}


void release_logger(Logger *logger)
{
    info_log(logger, "FREEING [LOG] RESOURCES.\n");
    release_file_system(logger->medium);

    // Free the dynamically-allocated memory
    if(logger != NULL)
    {
        free(logger);
    }
}


void info_log(Logger *logger, char *fmt, ...)
{
    LOG_COMMON_CODE(INSTAMSG_LOG_LEVEL_INFO)
}


void error_log(Logger *logger, char *fmt, ...)
{
    LOG_COMMON_CODE(INSTAMSG_LOG_LEVEL_ERROR)
}


void debug_log(Logger *logger, char *fmt, ...)
{
    LOG_COMMON_CODE(INSTAMSG_LOG_LEVEL_DEBUG)
}
