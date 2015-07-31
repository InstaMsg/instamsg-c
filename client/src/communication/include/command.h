/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

/*
 * Interface for sending/receiving bytes between the instamg-client (on the device) and the device-command-interface.
 *
 * Generally, the interface would be a serial-port, through which the instamsg-client can send the command-bytes, and
 * receive the command-response-bytes.
 */


#ifndef INSTAMSG_SERIAL
#define INSTAMSG_SERIAL

#include "./common.h"

typedef struct Command Command;
struct Command
{
    COMMUNICATION_INTERFACE(Command)
};

Command* get_new_command_interface(void *arg);
void release_command_interface(Command*);

#endif
