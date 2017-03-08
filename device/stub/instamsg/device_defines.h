#ifndef DEVICE_DEFINES
#define DEVICE_DEFINES

/*
 * By default.. we use the in-built functionality, which uses variable-argument lists.
 *
 * However, this implementation has shown to give problems for Harvard/RISC architectures, so such devices will need
 * to provide the reference to an implementation shipped with their compiler-environment.
 */

#define DEVICE_VERSION "1.0.0"

#define sg_sprintf                  DEFAULT_SPRINTF

#define USE_DEFAULT_MALLOC          1
#define sg_malloc                   DEFAULT_MALLOC
#define sg_free                     DEFAULT_FREE

#define PROSTR

#define MAX_BUFFER_SIZE             1000
#define READ_BUFFER_SIZE            MAX_BUFFER_SIZE

#define DATA_LOGGING_ENABLED        1

#define OTA_BUFFER_SIZE             500
#define OTA_PING_BUFFER_SIZE        500

#define MEDIA_STREAMING_ENABLED     0
#define FILE_LOGGING_ENABLED        0
#define FILE_SYSTEM_ENABLED         0
#define GSM_INTERFACE_ENABLED       0
#define SSL_ENABLED                 0
#define SOCKET_SSL_ENABLED          0

#define NTP_TIME_SYNC_PRESENT       1
#define GPS_TIME_SYNC_PRESENT       0
#define GSM_TIME_SYNC_PRESENT       0

#define SEND_GPS_LOCATION           0

#define ACQUIRE_THREAD_MUTEX
#define RELEASE_THREAD_MUTEX

#define ACQUIRE_LOG_MUTEX
#define RELEASE_LOG_MUTEX

#define MAX_MESSAGE_HANDLERS        5
#define AT_INTERFACE_ENABLED        0

#define DEFAULT_COMPULSORY_SOCKET_READ_AFTER_WRITE_TIMEOUT      "0"
#define DEFAULT_MAX_SECONDS_FOR_GPS_GSM_TIME_SYNC               "0"

#endif
