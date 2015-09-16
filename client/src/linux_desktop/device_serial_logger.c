/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

#include "./device_serial_logger.h"
#include "../common/include/globals.h"


/*
 * This method MUST connect the underlying medium (even if it means to retry continuously).
 */
void connect_underlying_serial_logger_medium_guaranteed(SerialLoggerInterface *serialLoggerInterface)
{
}


/*
 * This method writes first "len" bytes from "buffer" onto the serial-logger-interface.
 *
 * This is a blocking function. So, either of the following must hold true ::
 *
 * a)
 * All "len" bytes are written.
 * In this case, SUCCESS must be returned.
 *
 *                      OR
 * b)
 * An error occurred while writing.
 * In this case, FAILURE must be returned immediately.
 */
int serial_logger_write(SerialLoggerInterface* serialLoggerInterface, unsigned char* buffer, int len)
{
    return SUCCESS;
}


/*
 * This method MUST release the underlying medium (even if it means to retry continuously).
 */
void release_underlying_serial_logger_medium_guaranteed(SerialLoggerInterface *serialLoggerInterface)
{
}
