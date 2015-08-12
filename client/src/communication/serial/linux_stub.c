/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/



#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "instamsg_vendor.h"
#include "../../common/include/globals.h"


static int linux_serial_read(Serial *serial, unsigned char* buffer, int len, unsigned char guaranteed)
{
    return SUCCESS;
}


static int linux_serial_write(Serial *serial, unsigned char* buffer, int len)
{
    return SUCCESS;
}


void init_serial_interface(Serial *serial, void *arg)
{
    // Register read-callback.
	serial->read = linux_serial_read;

    // Register write-callback.
	serial->write = linux_serial_write;
}


void release_serial_interface(Serial *serial)
{
}
