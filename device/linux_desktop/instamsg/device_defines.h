#ifndef DEVICE_DEFINES
#define DEVICE_DEFINES

#define DEVICE_VERSION "2.3.0"

/*
 * We use the vanilla "sprintf" method for Linux.
 */
#include <stdio.h>
#define sg_sprintf sprintf



/*
 * We use the vanilla "malloc/free" methods for Linux.
 */
#define USE_DEFAULT_MALLOC      0

#include <stdlib.h>
#define sg_malloc               malloc
#define sg_free                 free

#define PROSTR

#define MAX_BUFFER_SIZE 1000
#define MAX_HEAP_SIZE (10 * (MAX_BUFFER_SIZE + HEADER_SIZE))

#define MODBUS_COMMAND_BUFFER_SIZE 150

#define MEDIA_STREAMING_ENABLED     1
#define FILE_SYSTEM_ENABLED         1
#define GSM_INTERFACE_ENABLED       0
#define SSL_ENABLED                 0

#endif
