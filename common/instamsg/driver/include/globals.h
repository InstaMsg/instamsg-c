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



#ifndef INSTAMSG_GLOBALS
#define INSTAMSG_GLOBALS

#define INSTAMSG_VERSION "3.0.0"

#include "device_defines.h"

enum returnCode
{
    SOCKET_READ_TIMEOUT = -3,
    BUFFER_OVERFLOW = -2,
    FAILURE = -1,
    SUCCESS = 0
};

enum ValueType
{
    STRING = 0,
    INTEGER
};


typedef struct KeyValuePairs KeyValuePairs;
struct KeyValuePairs
{
    char *key;
    char *value;
};


#define HTTP_FILE_DOWNLOAD_SUCCESS 200
#define HTTP_FILE_UPLOAD_SUCCESS 200

#define CONTENT_LENGTH PROSTR("Content-Length")
#define POST_BOUNDARY "-----------ThIs_Is_tHe_bouNdaRY_78564$!@"

#define SOCKET_READ     PROSTR("[SOCKET-READ] ")
#define SOCKET_WRITE    PROSTR("[SOCKET-WRITE] ")
#define ONE_TO_ONE      PROSTR("[ONE-TO-ONE] ")

#define SOCKET_NOT_AVAILABLE PROSTR("Could not connect to the socket at physical-layer ... not retrying")
#define MAX_CONN_ATTEMPTS_WITH_PHYSICAL_LAYER_FINE 5

#define NOTIFICATION_TOPIC PROSTR("instamsg/client/notifications")


/****************************************************************************************/
#if 1
#define INSTAMSG_HOST           PROSTR("device.instamsg.io")
#define INSTAMSG_HTTP_HOST      PROSTR("platform.instamsg.io")
#else
#define INSTAMSG_HOST           PROSTR("localhost")
#define INSTAMSG_HTTP_HOST      PROSTR("localhost")
#endif

#define SSL_ACTUALLY_ENABLED    PROSTR("SSL_ENABLED")
#define LOG_LEVEL               2

extern int INSTAMSG_PORT;
extern INSTAMSG_HTTP_PORT;
extern unsigned char sslEnabled;

#define NTP_PORT            123

#define SOCKET_READ_TIMEOUT_SECS 1
#define MAX_TRIES_ALLOWED_WHILE_READING_FROM_SOCKET_MEDIUM 1

#define ENABLE_DEBUG_PROCESSING 0
#define MAX_CLIENT_ID_SIZE 50
#define MAX_GSM_PROVISION_PARAM_SIZE 50
/***************************************************************************************/

static const char * const g_pcHex = "0123456789abcdef";

extern char LOG_GLOBAL_BUFFER[MAX_BUFFER_SIZE];

#include <stdarg.h>
void DEFAULT_SPRINTF(char *out, const char *format, ...);

#define NO_TIMEOUT 0

void startAndCountdownTimer(int seconds, unsigned char showRunningStatus);
void globalSystemInit(char *logFilePath);

#define MEM_ALLOC     PROSTR("[MEM-ALLOC] ")

extern unsigned char GLOBAL_BUFFER[MAX_BUFFER_SIZE];
#define RESET_GLOBAL_BUFFER memset(GLOBAL_BUFFER, 0, MAX_BUFFER_SIZE);

#define TOPIC_METADATA          PROSTR("instamsg/client/metadata")
#define TOPIC_SESSION_DATA      PROSTR("instamsg/client/session")
#define TOPIC_NETWORK_DATA      PROSTR("instamsg/client/signalinfo")
#define TOPIC_CONFIG_SEND       PROSTR("instamsg/client/config/clientToServer")

#define NETWORK_INFO_INTERVAL           300
extern int pingRequestInterval;
extern int compulsorySocketReadAfterMQTTPublishInterval;
#if MEDIA_STREAMING_ENABLED == 1
extern int mediaStreamingEnabledRuntime;
#endif

extern char messageBuffer[MAX_BUFFER_SIZE];


#if MEDIA_STREAMING_ENABLED == 1
extern unsigned char mediaStreamingErrorOccurred;
#endif

#ifdef DEBUG_MODE
extern char USER_LOG_FILE_PATH[MAX_BUFFER_SIZE];
extern char USER_DEVICE_UUID[MAX_BUFFER_SIZE];
#endif

extern int editableBusinessLogicInterval;

#define SYSTEM_WIDE_TEMP_FILE   "temp"
#endif
