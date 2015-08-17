/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

#include <stdio.h>

#include "instamsg_vendor.h"
#include "atoll_common.h"

#include "../../common/include/globals.h"


static int ar501_uart_read(SerialLoggerInterface *serialLoggerInterface, unsigned char* buffer, int len, unsigned char guaranteed)
{
    return SUCCESS;
}


static int ar501_uart_write(SerialLoggerInterface *serialLoggerInterface, unsigned char* buffer, int len)
{
    char finalStringToSend[MAX_BUFFER_SIZE] = {0};
    sprintf(finalStringToSend, "\r\n%s", buffer);

    AtollUart0_StringSend(finalStringToSend);

    return SUCCESS;
}


void init_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface, void *arg)
{
    // Register read-callback.
	serialLoggerInterface->read = ar501_uart_read;

    // Register write-callback.
	serialLoggerInterface->write = ar501_uart_write;
}


void release_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface)
{
}
