/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/



#include <string.h>
#include <stdarg.h>

#include "./include/instamsg.h"
#include "./include/log.h"
#include "./include/globals.h"

static char LOG_GLOBAL_BUFFER[MAX_BUFFER_SIZE];

#ifdef FILE_SYSTEM_INTERFACE_ENABLED
#define LOG_COMMON_CODE(level)                                                                          \
                                                                                                        \
    va_list argptr;                                                                                     \
    memset(LOG_GLOBAL_BUFFER, 0, MAX_BUFFER_SIZE);                                                      \
                                                                                                        \
    va_start(argptr, fmt);                                                                              \
    sg_varargs(LOG_GLOBAL_BUFFER, fmt, argptr);                                                         \
    va_end(argptr);                                                                                     \
                                                                                                        \
    strcat(LOG_GLOBAL_BUFFER, "\r\n");                                                                  \
                                                                                                        \
    if(instaMsg.serverLoggingEnabled == 1)                                                              \
    {                                                                                                   \
        MQTTPublish(&instaMsg,                                                                          \
                    instaMsg.serverLogsTopic,                                                           \
                    LOG_GLOBAL_BUFFER,                                                                  \
                    QOS0,                                                                               \
                    0,                                                                                  \
                    NULL,                                                                               \
                    MQTT_RESULT_HANDLER_TIMEOUT,                                                        \
                    0,                                                                                  \
                    0);                                                                                 \
    }                                                                                                   \
    else if(level <= currentLogLevel)                                                                   \
    {                                                                                                   \
        serial_logger_write((unsigned char *)LOG_GLOBAL_BUFFER, strlen(LOG_GLOBAL_BUFFER));             \
        fileLogger.fs.write(                                                                            \
                &(fileLogger.fs),                                                                       \
                (unsigned char *)LOG_GLOBAL_BUFFER, strlen(LOG_GLOBAL_BUFFER));                         \
    }
#else
#define LOG_COMMON_CODE(level)                                                                          \
                                                                                                        \
    va_list argptr;                                                                                     \
    memset(LOG_GLOBAL_BUFFER, 0, MAX_BUFFER_SIZE);                                                      \
                                                                                                        \
    va_start(argptr, fmt);                                                                              \
    sg_varargs(LOG_GLOBAL_BUFFER, fmt, argptr);                                                         \
    va_end(argptr);                                                                                     \
                                                                                                        \
    strcat(LOG_GLOBAL_BUFFER, "\r\n");                                                                  \
                                                                                                        \
    if(instaMsg.serverLoggingEnabled == 1)                                                              \
    {                                                                                                   \
        MQTTPublish(&instaMsg,                                                                          \
                    instaMsg.serverLogsTopic,                                                           \
                    LOG_GLOBAL_BUFFER,                                                                  \
                    QOS0,                                                                               \
                    0,                                                                                  \
                    NULL,                                                                               \
                    MQTT_RESULT_HANDLER_TIMEOUT,                                                        \
                    0,                                                                                  \
                    0);                                                                                 \
    }                                                                                                   \
    else if(level <= currentLogLevel)                                                                   \
    {                                                                                                   \
        serial_logger_write((unsigned char *)LOG_GLOBAL_BUFFER, strlen(LOG_GLOBAL_BUFFER));             \
    }
#endif


#ifdef FILE_SYSTEM_INTERFACE_ENABLED
void init_file_logger(FileLogger *fileLogger, void *arg)
{
    /* Here, physical medium is a file-system. */
	init_file_system(&(fileLogger->fs), arg);
}


void release_file_logger(FileLogger *fileLogger)
{
    info_log("FREEING [LOG] RESOURCES (FILE-BASED).\n");

    release_file_system(&(fileLogger->fs));
}
#endif


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
