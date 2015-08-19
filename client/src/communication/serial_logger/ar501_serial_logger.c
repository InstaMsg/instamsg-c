/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/


#include "instamsg_vendor.h"


/*
 * This method writes to the serial-logger-interface.
 *
 * This method is just for debugging-purposes.
 *
 * For example, in an embedded-device where UART is available, we could connect the UART to a serial-terminal on a desktop,
 * and see the logs there, which will help greatly in debugging.
 *
 * PLEASE NOTE THAT THIS METHOD MUST NOT BLOCK, for eg. if the UART-write buffers are full, then this method MUST NOT wait
 * for the buffers to become empty; instead, the method must return immediately.
 *
 * Note that the UART-write-buffers-full scenario is a very likely one (when there is no serial-cable attached between
 * embedded-device and laptop).
 *
 * If all "len" bytes are written, this method must return SUCCESS.
 * If all "len" bytes could not be written, this method must return FAILURE.
 */
static int ar501_serial_logger_write(SerialLoggerInterface *serialLoggerInterface, unsigned char* buffer, int len)
{
}


/*
 * NOTHING EXTRA NEEDS TO BE DONE HERE.
 */
void init_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface, void *arg)
{
    // Register write-callback.
	serialLoggerInterface->write = ar501_serial_logger_write;
}


/*
 * NOTHING EXTRA NEEDS TO BE DONE HERE.
 */
void release_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface)
{
}
