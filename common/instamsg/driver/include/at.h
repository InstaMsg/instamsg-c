#include "device_defines.h"

#if AT_INTERFACE_ENABLED == 1

#ifndef INSTAMSG_AT
#define INSTAMSG_AT


/*
 * Public-APIs.
 */
void run_simple_at_command_and_get_output(const char *command, int len, char *usefulOutput, int maxBufferLimit, const char *delimiter,
                                          unsigned char showCommandOutput, unsigned char strip);


/*
 * Internal-APIs.
 */
void init_at_interface();
void do_fire_at_command_and_get_output(const char *command, int len, char *usefulOutput, const char *delimiter);
void release_at_interface();
#endif

#else
typedef int just_to_make_compiler_happy;

#endif
