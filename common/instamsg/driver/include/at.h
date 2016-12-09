#include "device_defines.h"

#if AT_INTERFACE_ENABLED == 1

#ifndef INSTAMSG_AT
#define INSTAMSG_AT

/*
 * Public-APIs.
 */
void run_simple_at_command_and_get_output(const char *command, char *usefulOutput, const char *delimiter);


/*
 * Internal-APIs.
 */
void do_fire_at_command_and_get_output(const char *command, char *usefulOutput, const char *delimiter);
#endif

#endif
