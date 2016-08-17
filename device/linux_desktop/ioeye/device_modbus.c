#include "device_modbus.h"

#include "../../../common/ioeye/include/modbus.h"

#define PORT_NAME       PROSTR("/dev/ttyUSB0")

/*
 * This method initializes and connects to the Modbus-interface.
 */
void connect_underlying_modbus_medium_guaranteed(Modbus *modbus)
{
    connect_serial_port(&(modbus->fd), PORT_NAME);
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
        int bytes_received = 0;
        while((bytes_received < responseBytesLength) && (time_fine_for_time_limit_function() == 1))
        {
            bytes_received = bytes_received + read(modbus->fd, responseByteBuffer + bytes_received, responseBytesLength - bytes_received);
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
    if(modbus->fd < 0)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "Modbus-FD is less than zero");
        error_log(LOG_GLOBAL_BUFFER);

        return FAILURE;
    }
    else
    {
        close(modbus->fd);

        return SUCCESS;
    }
}
