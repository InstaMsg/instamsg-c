/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

#include "instamsg_vendor.h"
#include "../../common/include/globals.h"


static int linux_command_stub_read(Command* command, unsigned char* buffer, int len, unsigned char guaranteed)
{
    return SUCCESS;
}


static int linux_command_stub_write(Command* fs, unsigned char* buffer, int len)
{
    return SUCCESS;
}


void init_command_interface(Command *command, void *arg)
{
    // Register read-callback.
	command->read = linux_command_stub_read;

    // Register write-callback.
	command->write = linux_command_stub_write;
}


void release_command_interface(Command *command)
{
}
