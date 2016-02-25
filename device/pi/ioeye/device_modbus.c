#include "../../../common/instamsg/driver/include/globals.h"

#include "./device_modbus.h"

/*
 * This method initializes and connects to the Modbus-interface.
 */
void connect_underlying_modbus_medium_guaranteed(Modbus *modbus)
{
}


/*
 * This method ::
 *
 * a)
 * Sends the command to the modbus-interface.
 *
 * b)
 * Receives "responseBytesLength" number of bytes from the interface.
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
 *
 * So, it is advisable (but not necessary) to wrap this function in a watchdog by the device-impementors.
 * If the modbus-response (equal to "responseBytesLength") is not available soonish, the device should be reset.
 */
int modbus_send_command_and_read_response_sync(Modbus *modbus,
                                               unsigned char *commandBytes,
                                               int commandBytesLength,
                                               unsigned char *responseByteBuffer,
                                               int responseBytesLength)
{
    return FAILURE;
}


/*
 * This method cleans up the modbus-interface.
 */
void release_underlying_modbus_medium_guaranteed(Modbus *modbus)
{
}
