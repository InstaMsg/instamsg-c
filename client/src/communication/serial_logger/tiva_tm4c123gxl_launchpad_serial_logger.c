/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/


#include "inc/hw_memmap.h"

#include "instamsg_vendor.h"
#include "uart_utils.h"
#include "../../common/include/globals.h"


static int tiva_serial_logger_write(SerialLoggerInterface* serialLoggerInterface, unsigned char* buffer, int len)
{
    UARTSend(UART0_BASE, buffer, len);
    return SUCCESS;
}


void init_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface, void *arg)
{
    // No init-required here.
    // Just to be absolutely sure, we are doing all hardware-initialization in SYSTEM_GLOBAL_INIT

    // Register write-callback.
	serialLoggerInterface->write = tiva_serial_logger_write;
}


void release_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface)
{
    /*
     * Nothing to be done as such.
     * Multiple re-inits (without any so-called previous cleanups) SHOULD not cause any issues.
     */
}
