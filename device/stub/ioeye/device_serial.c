#include "device_serial.h"

#include "../../ioeye/include/serial.h"


/*
 * This method initializes and connects to the serial-interface.
 */
void connect_underlying_serial_medium_guaranteed(Serial *serial)
{
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
    return FAILURE;
}


/*
 * This method cleans up the serial-interface.
 *
 * Returns SUCCESS, if the interface was closed successfully.
 *         FAILURE, if the interface could not be closed successfully.
 */
int release_underlying_serial_medium_guaranteed(Serial *serial)
{
    return FAILURE;
}
