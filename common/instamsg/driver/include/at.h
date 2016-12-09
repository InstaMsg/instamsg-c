#include "device_defines.h"

#if AT_INTERFACE_ENABLED == 1

#ifndef INSTAMSG_AT
#define INSTAMSG_AT

extern unsigned char showCommandOutput;


/*
 * Public-APIs.
 */
void run_simple_at_command_and_get_output(const char *command, char *usefulOutput, const char *delimiter);


/*
 * Internal-APIs.
 */
void init_at_interface();
void do_fire_at_command_and_get_output(const char *command, char *usefulOutput, const char *delimiter);
void release_at_interface();
#endif

#else
typedef int just_to_make_compiler_happy;

#endif
