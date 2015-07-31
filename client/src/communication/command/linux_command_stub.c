/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

#include <stdlib.h>

#include "../include/command.h"
#include "../../common/include/globals.h"


static int linux_command_stub_read(Command* command, unsigned char* buffer, int len)
{
    return SUCCESS;
}


static int linux_command_stub_write(Command* fs, unsigned char* buffer, int len)
{
    return SUCCESS;
}


Command* get_new_command_interface(void *arg)
{
    Command *command = (Command*)malloc(sizeof(Command));

    // Register read-callback.
	command->read = linux_command_stub_read;

    // Register write-callback.
	command->write = linux_command_stub_write;

    return command;
}


void release_command_interface(Command *command)
{
    // Free the dynamically-allocated memory
    if(command != NULL)
    {
        free(command);
    }
}
