#include "device_defines.h"

#if AT_INTERFACE_ENABLED == 1

/*
 * Initializes the AT-interface.
 */
void init_at_interface()
{
}


/*
 * This command fires the "command" on AT-interface, and gets the response in "usefulOutput"
 * as soon as "delimiter" suffix arrives.
 *
 * If the above does not happen exactly within 10 seconds, the system is reset.
 */
void do_fire_at_command_and_get_output(const char *command, char *usefulOutput, const char *delimiter)
{
}


/*
 * Releases the AT-interface.
 */
void release_at_interface()
{
}
#endif
