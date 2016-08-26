#ifndef DEVICE_DEFINES
#define DEVICE_DEFINES

#define DEVICE_VERSION "2.20.1"

/*
 * We use the vanilla "sprintf" method for Linux.
 */
#include <stdio.h>
#define sg_sprintf sprintf



/*
 * We use the vanilla "malloc/free" methods for Linux.
 */
#define USE_DEFAULT_MALLOC          0

#include <stdlib.h>
#define sg_malloc                   malloc
#define sg_free                     free

#define PROSTR

#define FILE_STRUCT                 FILE
#define FILE_OPEN                   fopen
#define FILE_CLOSE                  fclose
#define FILE_END_ID                 EOF
#define FILE_GETC                   fgetc
#define FILE_PUTC                   fputc

#define MAX_BUFFER_SIZE 1000
#define MAX_HEAP_SIZE (10 * (MAX_BUFFER_SIZE + HEADER_SIZE))

#define DATA_LOGGING_ENABLED        1
#define MAX_DATA_LOGGER_SIZE_BYTES  100000

#define SERIAL_COMMANDS_BUFFER_SIZE 600
#define NUM_CLASSICAL_MODBUS_PORTS  1
#define NUM_RESPONSE_LF_TERM_PORTS  0

#define OTA_BUFFER_SIZE             500
#define OTA_PING_BUFFER_SIZE        10000


#define MEDIA_STREAMING_ENABLED     1
#define FILE_LOGGING_ENABLED        1
#define FILE_SYSTEM_ENABLED         1
#define GSM_INTERFACE_ENABLED       0
#define SSL_ENABLED                 0

#define NTP_TIME_SYNC_PRESENT       0
#define GPS_TIME_SYNC_PRESENT       0
#define GSM_TIME_SYNC_PRESENT       0

#define SEND_GPS_LOCATION           0

#define ACQUIRE_THREAD_MUTEX
#define RELEASE_THREAD_MUTEX

#define ACQUIRE_LOG_MUTEX
#define RELEASE_LOG_MUTEX

#endif
