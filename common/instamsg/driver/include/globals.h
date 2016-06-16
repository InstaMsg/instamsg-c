#ifndef INSTAMSG_GLOBALS
#define INSTAMSG_GLOBALS

#define INSTAMSG_VERSION "2.8.12"

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

#define INSTAMSG_HOST       PROSTR("device.instamsg.io")

#if  SSL_ENABLED == 1
#define INSTAMSG_PORT       8883
#else
#define INSTAMSG_PORT       1883
#endif

#define INSTAMSG_HTTP_HOST  PROSTR("platform.instamsg.io")

#if  SSL_ENABLED == 1
#define INSTAMSG_HTTP_PORT  443
#else
#define INSTAMSG_HTTP_PORT  80
#endif

#define LOG_LEVEL           2

#else

#define INSTAMSG_HOST       PROSTR("localhost")

#if  SSL_ENABLED == 1
#define INSTAMSG_PORT       8883
#else
#define INSTAMSG_PORT       1883
#endif

#define INSTAMSG_HTTP_HOST  PROSTR("localhost")

#if  SSL_ENABLED == 1
#define INSTAMSG_HTTP_PORT  8601
#else
#define INSTAMSG_HTTP_PORT  8600
#endif

#define LOG_LEVEL           2

#endif


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

#define LOG_FILE_PATH           PROSTR("/home/sensegrow/instamsg.log")

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
