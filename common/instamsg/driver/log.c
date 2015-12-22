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



#ifdef FILE_SYSTEM_INTERFACE_ENABLED
#define LOG_COMMON_CODE(log, level)                                                                     \
                                                                                                        \
    strcat(log, "\r\n");                                                                                \
                                                                                                        \
    if(instaMsg.serverLoggingEnabled == 1)                                                              \
    {                                                                                                   \
        publish(instaMsg.serverLogsTopic,                                                               \
                log,                                                                                    \
                QOS0,                                                                                   \
                0,                                                                                      \
                NULL,                                                                                   \
                MQTT_RESULT_HANDLER_TIMEOUT,                                                            \
                0);                                                                                     \
    }                                                                                                   \
    else if(level <= currentLogLevel)                                                                   \
    {                                                                                                   \
        serial_logger_write((unsigned char *)log, strlen(log));                                         \
        fileLogger.fs.write(&(fileLogger.fs), (unsigned char *)log, strlen(log));                       \
    }                                                                                                   \
    memset(LOG_GLOBAL_BUFFER, 0, sizeof(LOG_GLOBAL_BUFFER));
#else
#define LOG_COMMON_CODE(log, level)                                                                     \
                                                                                                        \
    strcat(log, "\r\n");                                                                                \
                                                                                                        \
    if(instaMsg.serverLoggingEnabled == 1)                                                              \
    {                                                                                                   \
        publish(instaMsg.serverLogsTopic,                                                               \
                log,                                                                                    \
                QOS0,                                                                                   \
                0,                                                                                      \
                NULL,                                                                                   \
                MQTT_RESULT_HANDLER_TIMEOUT,                                                            \
                0);                                                                                     \
    }                                                                                                   \
    else if(level <= currentLogLevel)                                                                   \
    {                                                                                                   \
        serial_logger_write((unsigned char *)log, strlen(log));                                         \
    }                                                                                                   \
    memset(LOG_GLOBAL_BUFFER, 0, sizeof(LOG_GLOBAL_BUFFER));
#endif


#ifdef FILE_SYSTEM_INTERFACE_ENABLED
void init_file_logger(FileLogger *fileLogger, void *arg)
{
    /* Here, physical medium is a file-system. */
	init_file_system(&(fileLogger->fs), arg);
}


void release_file_logger(FileLogger *fileLogger)
{
    sg_sprintf(LOG_GLOBAL_BUFFER, "FREEING [LOG] RESOURCES (FILE-BASED).\n");

    release_file_system(&(fileLogger->fs));
}
#endif


void info_log(char *log)
{
    LOG_COMMON_CODE(log, INSTAMSG_LOG_LEVEL_INFO)
}


void error_log(char *log)
{
    LOG_COMMON_CODE(log, INSTAMSG_LOG_LEVEL_ERROR)
}


void debug_log(char *log)
{
    LOG_COMMON_CODE(log, INSTAMSG_LOG_LEVEL_DEBUG)
}
