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
#include "./include/sg_stdlib.h"


#define EXPIRY_SECONDS "600"

int currentLogLevel;

#if FILE_SYSTEM_ENABLED == 1
#define LOG_COMMON_CODE(log, level)                                                                     \
                                                                                                        \
    strcat(log, "\r\n");                                                                                \
                                                                                                        \
    if((instaMsg.serverLoggingEnabled == 1) && (level <= currentLogLevel))                              \
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
    if((instaMsg.serverLoggingEnabled == 1) && (level <= currentLogLevel))                              \
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


#if FILE_SYSTEM_ENABLED == 1
void init_file_logger(FileLogger *fileLogger, void *arg)
{
    /* Here, physical medium is a file-system. */
	init_file_system(&(fileLogger->fs), arg);
}


void release_file_logger(FileLogger *fileLogger)
{
    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("FREEING [LOG] RESOURCES (FILE-BASED).\n"));

    release_file_system(&(fileLogger->fs));
}
#endif


static void processServerLoggingMetadata()
{
    InstaMsg * c = &instaMsg;

    if(c->serverLoggingEnabled == 1)
    {
        if(c->serverLogsStartTime == c->FRESH_SERVER_LOGS_TIME)
        {
            c->serverLogsStartTime = getCurrentTick();
        }

        /*
		 * We allow the server-logs only for 10-minutes.
		 */
        if(c->serverLogsStartTime != c->FRESH_SERVER_LOGS_TIME)
        {
            long currentTick = getCurrentTick();
            if(currentTick > ( (c->serverLogsStartTime) + sg_atoi(EXPIRY_SECONDS) ))
            {
                c->serverLogsStartTime = c->FRESH_SERVER_LOGS_TIME;
                c->serverLoggingEnabled = 0;

                publish(c->serverLogsTopic,
			            "****** DISABLING SERVER LOGS, as " EXPIRY_SECONDS " seconds have passed ******",
                        QOS0,
                        0,
                        NULL,
                        MQTT_RESULT_HANDLER_TIMEOUT,
                        0);                                                                                     \
			}
        }
    }
}


void info_log(char *log)
{
    processServerLoggingMetadata();
    LOG_COMMON_CODE(log, INSTAMSG_LOG_LEVEL_INFO)
}


void error_log(char *log)
{
    processServerLoggingMetadata();
    LOG_COMMON_CODE(log, INSTAMSG_LOG_LEVEL_ERROR)
}


void debug_log(char *log)
{
    processServerLoggingMetadata();
    LOG_COMMON_CODE(log, INSTAMSG_LOG_LEVEL_DEBUG)
}
