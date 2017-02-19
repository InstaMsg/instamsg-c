#include "device_serial.h"

#include "../../../common/ioeye/include/serial.h"
#include "../common/serial_port_utils.h"

#include <termios.h>
#include <string.h>

#define PORT_NAME       PROSTR("/dev/ttyUSB0")

static pthread_t tid;

static unsigned char response_thread_started;

static volatile unsigned char *responseBuffer;
static volatile int bytesToRead;
static volatile int responseBytesSoFar;

static volatile unsigned char readResponse;

static volatile Serial *global_serial;
static volatile unsigned char *global_commandBytes;
static volatile int global_commandBytesLength;
static volatile unsigned char global_serial_delimiter;

static volatile int maxTriesForSerialResponse;

static void* serial_poller_func(void *arg)
{
    while(1)
    {
        if(readResponse == 0)
        {
            startAndCountdownTimer(1, 0);
            continue;
        }

        read(global_serial->fd, ((unsigned char*)responseBuffer) + responseBytesSoFar, 1);
        responseBytesSoFar++;

        if(responseBuffer[0] == 0)
        {
            responseBytesSoFar = 0;
        }
        else if(bytesToRead > 0)
        {
            /*
             * Fixed-bytes case.
             */
            if(responseBytesSoFar == bytesToRead)
            {
                readResponse = 0;
            }
        }
        else if(responseBuffer[responseBytesSoFar - 1] == global_serial_delimiter)
        {
            /*
             * Delimiter case.
             */
            if((responseBytesSoFar == 1) && (maxTriesForSerialResponse < 3))
            {
                /*
                 * Handle when empty response is received.
                 */
                maxTriesForSerialResponse++;
		        responseBytesSoFar = 0;
    		    write(global_serial->fd, (unsigned char*)global_commandBytes, global_commandBytesLength);
	        }
            else
            {
                /*
                 * We are done.
                 */
                readResponse = 0;
            }
        }
    }

    return NULL;
}


/*
 * This method initializes and connects to the serial-interface.
 */
void connect_underlying_serial_medium_guaranteed(Serial *serial)
{
    if(response_thread_started == 0)
    {
        pthread_create(&tid, NULL, serial_poller_func, NULL);
        response_thread_started = 1;
    }

    connect_serial_port(&(serial->fd), PORT_NAME, serial->serialParams, 0);
}


/*
 * This method ::
 *
 * a)
 * Sends the command to the serial-interface.
 *
 * b)
 * Response is received in "responseByteBuffer", while the following method returns 1 ::
 *
 *                                          time_fine_for_time_limit_function()
 *
 * Number of bytes to-be-actually read is determined from the following ::
 *
 *      * If #*responseBytesLength > 0, then #*responseBytesLength bytes are read.
 *
 *      * Else the command-response is delimited by a delimiter, and the device needs
 *        to handle it appropriately, AND *responseBytesLength MUST BE SET APPROPRIATELY.
 *
 *
 *
 * The function must return exactly one of the following ::
 *
 * *
 * SUCCESS, if everything went fine.
 *
 * *
 * FAILURE, else.
 */
int serial_send_command_and_read_response_sync(Serial *serial,
                                               unsigned char *commandBytes,
                                               int commandBytesLength,
                                               unsigned char *responseByteBuffer,
                                               int *responseBytesLength)
{
    global_serial = serial;
    global_commandBytes = commandBytes;
    global_commandBytesLength = commandBytesLength;
    global_serial_delimiter = sg_atoi(serial->serialDelimiter);

    write(global_serial->fd, (unsigned char*)global_commandBytes, global_commandBytesLength);

    {
        responseBuffer = responseByteBuffer;
        bytesToRead = *responseBytesLength;
        maxTriesForSerialResponse = 0;
        responseBytesSoFar = 0;

        readResponse = 1;

        while((readResponse == 1) && (time_fine_for_time_limit_function() == 1))
        {
            startAndCountdownTimer(1, 0);
        }

        if(readResponse == 0)
        {
            *responseBytesLength = responseBytesSoFar;
        }

        /*
         * Stop the poller-thread to read unnecessarily.
         */
        readResponse = 0;
    }

    return SUCCESS;
}


/*
 * This method cleans up the modbus-interface.
 *
 * Returns SUCCESS, if the interface was closed successfully.
 *         FAILURE, if the interface could not be closed successfully.
 */
int release_underlying_serial_medium_guaranteed(Serial *serial)
{
    return disconnect_serial_port(serial->fd);
}
