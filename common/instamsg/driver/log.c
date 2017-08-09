/*******************************************************************************
 *
 * Copyright (c) 2014 SenseGrow, Inc.
 *
 * SenseGrow Internet of Things (IoT) Client Frameworks
 * http://www.sensegrow.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in 
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished to do so, 
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all 
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.

 * Contributors:
 *    Ajay Garg <ajay.garg@sensegrow.com>
 *******************************************************************************/



#include <string.h>
#include <stdarg.h>

#include "./include/instamsg.h"
#include "./include/log.h"
#include "./include/globals.h"
#include "./include/sg_stdlib.h"
#include "./include/file_system.h"


#define EXPIRY_SECONDS "600"

#if FILE_LOGGING_ENABLED == 1
FileLogger fileLogger;
#endif

int currentLogLevel;

#if FILE_LOGGING_ENABLED == 1
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
        if(debugLoggingEnabled == 1)                                                                    \
        {                                                                                               \
            serial_logger_write((unsigned char *)log, strlen(log));                                     \
            sg_file_write(&(fileLogger.fs), (unsigned char *)log, strlen(log));                         \
        }                                                                                               \
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
        if(debugLoggingEnabled == 1)                                                                    \
        {                                                                                               \
            serial_logger_write((unsigned char *)log, strlen(log));                                     \
        }                                                                                               \
    }                                                                                                   \
    memset(LOG_GLOBAL_BUFFER, 0, sizeof(LOG_GLOBAL_BUFFER));
#endif


#if FILE_LOGGING_ENABLED == 1
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

    ACQUIRE_LOG_MUTEX
    LOG_COMMON_CODE(log, INSTAMSG_LOG_LEVEL_INFO)
    RELEASE_LOG_MUTEX
}


void error_log(char *log)
{
    processServerLoggingMetadata();

    ACQUIRE_LOG_MUTEX
    LOG_COMMON_CODE(log, INSTAMSG_LOG_LEVEL_ERROR)
    RELEASE_LOG_MUTEX
}


void debug_log(char *log)
{
    processServerLoggingMetadata();

    ACQUIRE_LOG_MUTEX
    LOG_COMMON_CODE(log, INSTAMSG_LOG_LEVEL_DEBUG)
    RELEASE_LOG_MUTEX
}
