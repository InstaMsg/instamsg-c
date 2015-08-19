/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/



#include "instamsg_vendor.h"
#include "../../common/include/globals.h"


static int linux_serial_logger_write(SerialLoggerInterface *serialLoggerInterface, unsigned char* buffer, int len)
{
    return SUCCESS;
}


void init_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface, void *arg)
{
    // Register write-callback.
	serialLoggerInterface->write = linux_serial_logger_write;
}


void release_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface)
{
}
