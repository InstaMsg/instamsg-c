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




#ifndef DEVICE_DEFINES
#define DEVICE_DEFINES

#define DEVICE_VERSION "11.5.0"

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
#define FILE_READ_LINE              fread
#define FILE_WRITE_LINE             fwrite
#define FILE_DELETE                 remove
#define FILE_RENAME                 rename
#define FILE_FLUSH                  fflush

#define MAX_BUFFER_SIZE             5000
#define READ_BUFFER_SIZE            20000

#define DATA_LOGGING_ENABLED        1
#define MAX_DATA_LOGGER_SIZE_BYTES  100000

#define SERIAL_COMMANDS_BUFFER_SIZE 600

#define MAX_PORTS_ALLOWED           1

#define OTA_BUFFER_SIZE             500
#define OTA_PING_BUFFER_SIZE        10000


#define MEDIA_STREAMING_ENABLED                                 1
#define FILE_LOGGING_ENABLED                                    1
#define FILE_SYSTEM_ENABLED                                     1

#define GSM_INTERFACE_ENABLED       0
#define SSL_ENABLED                 1
#define SOCKET_SSL_ENABLED          0

#if SSL_ENABLED == 1
#define SSL_WIRE_BUFFER_SIZE        MAX_BUFFER_SIZE
#endif

#define NTP_TIME_SYNC_PRESENT       1
#define GPS_TIME_SYNC_PRESENT       0
#define GSM_TIME_SYNC_PRESENT       0

#define SEND_GPS_LOCATION           0

#define ACQUIRE_THREAD_MUTEX
#define RELEASE_THREAD_MUTEX

#define ACQUIRE_LOG_MUTEX
#define RELEASE_LOG_MUTEX

#define I386_ONLY
#define HAVE_LONG_LONG              0

#define MAX_MESSAGE_HANDLERS        50
#define AT_INTERFACE_ENABLED        0

#define SERIAL_RESPONSE_TIMEOUT_SECS    10

#define DEFAULT_COMPULSORY_SOCKET_READ_AFTER_WRITE_TIMEOUT  "3"
#define DEFAULT_NTP_SERVER                                  ""

#define ENSURE_EXPLICIT_TIME_SYNC       0
#define DEFAULT_SSL_ENABLED             "1"
#define MAX_CYCLES_TO_WAIT_FOR_PUBACK   20

#define HTTP_PROXY_ENABLED              1
#define SEND_POWER_INFORMATION          1

#define CRON_ENABLED                    1
#define DEFAULT_CRON_CONFIG             ""

#define COMPULSORY_NTP_SYNC             1

#define SEPARATOR                       "/"

#define INSTAMSG_HOST                   "device.instamsg.io"

#endif
