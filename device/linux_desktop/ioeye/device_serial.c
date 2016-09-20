#include "device_serial.h"

#include "../../../common/ioeye/include/serial.h"
#include "../common/serial_port_utils.h"

#include <termios.h>
#include <string.h>

#define PORT_NAME       PROSTR("/dev/ttyUSB0")

static pthread_t tid;

static unsigned char response_thread_started;

static unsigned char *responseBuffer;
static int bytesToRead;
static int responseBytesSoFar;

static int serial_fd;
static unsigned char readResponse;

static void* serial_poller_func(void *arg)
{
    while(1)
    {
        if(readResponse == 0)
        {
            startAndCountdownTimer(1, 0);
            continue;
        }

        read(serial_fd, responseBuffer + responseBytesSoFar, 1);
        responseBytesSoFar++;

        if(responseBytesSoFar == bytesToRead)
        {
            readResponse = 0;
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
    write(serial->fd, commandBytes, commandBytesLength);

    {
        responseBuffer = responseByteBuffer;
        bytesToRead = *responseBytesLength;
        responseBytesSoFar = 0;

        serial_fd = serial->fd;
        readResponse = 1;

        while((readResponse == 1) && (time_fine_for_time_limit_function() == 1))
        {
            startAndCountdownTimer(1, 0);
        }
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
