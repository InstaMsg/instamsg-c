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

char formatted_string[MAX_BUFFER_SIZE];

#ifdef FILE_SYSTEM_INTERFACE_ENABLED
#define LOG_COMMON_CODE(level)                                                                          \
                                                                                                        \
    SG_MEMSET(formatted_string, 0, MAX_BUFFER_SIZE)                                                     \
    register int *varg = (int *)(&fmt);                                                                 \
    char *firstLevel = &(formatted_string[0]);                                                          \
    sg_print(&firstLevel, varg);                                                                        \
    strcat(formatted_string, "\n");                                                                     \
                                                                                                        \
    if(instaMsg.serverLoggingEnabled == 1)                                                              \
    {                                                                                                   \
        MQTTPublish(&instaMsg,                                                                          \
                    instaMsg.serverLogsTopic,                                                           \
                    formatted_string,                                                                   \
                    QOS0,                                                                               \
                    0,                                                                                  \
                    NULL,                                                                               \
                    MQTT_RESULT_HANDLER_TIMEOUT,                                                        \
                    0,                                                                                  \
                    0);                                                                                 \
    }                                                                                                   \
    else if(level <= currentLogLevel)                                                                   \
    {                                                                                                   \
        serialLogger.serialLoggerInterface.write(                                                       \
                &(serialLogger.serialLoggerInterface),                                                  \
                (unsigned char*)formatted_string, strlen(formatted_string));                            \
        fileLogger.fs.write(&(fileLogger.fs), formatted_string, strlen(formatted_string));              \
    }
#else
#define LOG_COMMON_CODE(level)                                                                          \
                                                                                                        \
    SG_MEMSET(formatted_string, 0, MAX_BUFFER_SIZE)                                                     \
    register int *varg = (int *)(&fmt);                                                                 \
    char *firstLevel = &(formatted_string[0]);                                                          \
    sg_print(&firstLevel, varg);                                                                        \
    strcat(formatted_string, "\n");                                                                     \
                                                                                                        \
    if(instaMsg.serverLoggingEnabled == 1)                                                              \
    {                                                                                                   \
        MQTTPublish(&instaMsg,                                                                          \
                    instaMsg.serverLogsTopic,                                                           \
                    formatted_string,                                                                   \
                    QOS0,                                                                               \
                    0,                                                                                  \
                    NULL,                                                                               \
                    MQTT_RESULT_HANDLER_TIMEOUT,                                                        \
                    0,                                                                                  \
                    0);                                                                                 \
    }                                                                                                   \
    else if(level <= currentLogLevel)                                                                   \
    {                                                                                                   \
        serialLogger.serialLoggerInterface.write(                                                       \
                &(serialLogger.serialLoggerInterface),                                                  \
                (unsigned char*)formatted_string, strlen(formatted_string));                            \
    }
#endif


#ifdef FILE_SYSTEM_INTERFACE_ENABLED
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
#endif

void init_serial_logger(SerialLogger *serialLogger, void *arg)
{
    // Here, physical medium is a serial-interface.
	init_serial_logger_interface(&(serialLogger->serialLoggerInterface), arg);
}


void release_serial_logger(SerialLogger *serialLogger)
{
    info_log("FREEING [LOG] RESOURCES (SERIAL-BASES).");

    release_serial_logger_interface(&(serialLogger->serialLoggerInterface));
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
