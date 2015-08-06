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

#define LOG_COMMON_CODE(level)                                                                      \
    if(level > currentLogLevel)                                                                     \
        return;                                                                                     \
                                                                                                    \
    unsigned char formatted_string[MAX_BUFFER_SIZE] = {0};                                          \
    va_list argptr;                                                                                 \
                                                                                                    \
    va_start(argptr, fmt);                                                                          \
    vsnprintf(formatted_string, MAX_BUFFER_SIZE, fmt, argptr);                                      \
    va_end(argptr);                                                                                 \
                                                                                                    \
    strcat(formatted_string, "\n");                                                                 \
    (*logger_write_func)(logger_medium, formatted_string, strlen(formatted_string));


void init_file_logger(FileLogger *fileLogger, void *arg)
{
    // Here, physical medium is a file-system.
	init_file_system(&(fileLogger->fs), arg);
}


void release_file_logger(FileLogger *fileLogger)
{
    info_log("FREEING [LOG] RESOURCES (FILE-BASED).\n");

    release_file_system(&(fileLogger->fs));
}


void init_serial_logger(SerialLogger *serialLogger, void *arg)
{
    // Here, physical medium is a serial-interface.
	init_serial_interface(&(serialLogger->serial), arg);
}


void release_serial_logger(SerialLogger *serialLogger)
{
    info_log("FREEING [LOG] RESOURCES (SERIAL-BASES).\n");

    release_serial_interface(&(serialLogger->serial));
}


void info_log(char *fmt, ...)
{
    LOG_COMMON_CODE(INSTAMSG_LOG_LEVEL_INFO)
}


void error_log(char *fmt, ...)
{
    LOG_COMMON_CODE(INSTAMSG_LOG_LEVEL_ERROR)
}


void debug_log(char *fmt, ...)
{
    LOG_COMMON_CODE(INSTAMSG_LOG_LEVEL_DEBUG)
}
