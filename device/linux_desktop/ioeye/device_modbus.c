#include "device_modbus.h"

#include "../../../common/ioeye/include/modbus.h"
#include "../common/serial_port_utils.h"

#include <termios.h>
#include <string.h>

#define PORT_NAME       PROSTR("/dev/ttyUSB0")

static pthread_t tid;

static unsigned char response_thread_started;

static unsigned char *responseBuffer;
static int bytesToRead;
static int responseBytesSoFar;

static int modbus_fd;
static unsigned char readResponse;

static void* modbus_poller_func(void *arg)
{
    while(1)
    {
        if(readResponse == 0)
        {
            startAndCountdownTimer(1, 0);
            continue;
        }

        read(modbus_fd, responseBuffer + responseBytesSoFar, 1);
        responseBytesSoFar++;

        if(responseBytesSoFar == bytesToRead)
        {
            readResponse = 0;
        }
    }

    return NULL;
}


/*
 * This method initializes and connects to the Modbus-interface.
 */
void connect_underlying_modbus_medium_guaranteed(Modbus *modbus)
{
    if(response_thread_started == 0)
    {
        pthread_create(&tid, NULL, modbus_poller_func, NULL);
        response_thread_started = 1;
    }

    connect_serial_port(&(modbus->fd), PORT_NAME, B9600, 0, CS8, 1);
}


/*
 * This method ::
 *
 * a)
 * Sends the command to the modbus-interface.
 *
 * b)
 * Receives "responseBytesLength" number of bytes from the interface, while the following method returns 1 ::
 *
 *                                          time_fine_for_time_limit_function()
 *
 * Note that the calling-function must provide the number of bytes (responseBytesLength), since
 * the number of expected bytes is always calculatable from the sending-command itself.
 *
 *
 * The function must return exactly one of the following ::
 *
 * *
 * SUCCESS, if everything went fine.
 *
 * *
 * FAILURE, if "responseBytesLength" could not be received.
 *
 *
 * In general, modbus-interface (unlike a network-interface) is not expected to take too long in returning the response.
 * It will either return the response quickly, or will never.
 */
int modbus_send_command_and_read_response_sync(Modbus *modbus,
                                               unsigned char *commandBytes,
                                               int commandBytesLength,
                                               unsigned char *responseByteBuffer,
                                               int responseBytesLength)
{
    write(modbus->fd, commandBytes, commandBytesLength);

    {
        responseBuffer = responseByteBuffer;
        bytesToRead = responseBytesLength;
        responseBytesSoFar = 0;

        modbus_fd = modbus->fd;
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
int release_underlying_modbus_medium_guaranteed(Modbus *modbus)
{
    return disconnect_serial_port(modbus->fd);
}
