#include "./include/at.h"
#include "./include/watchdog.h"
#include "./include/sg_stdlib.h"
#include "./include/log.h"

#include <string.h>

char withoutTerminatedCommand[200];

#if AT_INTERFACE_ENABLED == 1
static void do_run_simple_at_command_and_get_output_with_timeout(const char *command, int len, char *usefulOutput, int maxBufferLimit,
                                                                 const char *delimiter, unsigned char showCommandOutput, unsigned char strip,
                                                                 int timeout)

{
    unsigned char watchdog_enable_required = 0;
    int offset = 0;

    memset(usefulOutput, 0, maxBufferLimit);

    if(watchdog_active == 0)
    {
        watchdog_enable_required = 1;
        watchdog_reset_and_enable(timeout, (char*)command, 1);
    }

    do_fire_at_command_and_get_output(command, len, usefulOutput, delimiter);
    if(strip == 1)
    {
        /*
         * a)
         * The "usefulOutput" is guaranteed to contain the "delimiter", and that too at the very end.
         *
         * b)
         * Also, whenever "strip" is 1, we assume that the "usefulOutput" is alphanumeric, so we can do all normal "string"-operations.
         */

        /*
         * Firstly, strip the delimiter.
         */
        offset = ((int) (strlen(usefulOutput))) - ((int) (strlen(delimiter)));
        if(offset > 0)
        {
            memset(usefulOutput + offset, 0, strlen(delimiter));
        }

        /*
         * Secondly, strip the whitespaces.
         */
        strip_leading_and_trailing_white_paces(usefulOutput);
    }

    if(watchdog_enable_required == 1)
    {
        watchdog_disable(NULL, NULL);
    }

    if(showCommandOutput == 1)
    {
		int commandLength = strlen(command);
		
		memset(withoutTerminatedCommand, 0, sizeof(withoutTerminatedCommand));
		if(command[commandLength - 1] == '\r')
		{
			memcpy(withoutTerminatedCommand, command, commandLength - 1);
		}
		else
		{
			memcpy(withoutTerminatedCommand, command, commandLength);
		}
		
        sg_sprintf(LOG_GLOBAL_BUFFER, "Command = [%s], Output = [%s]", withoutTerminatedCommand, usefulOutput);
        info_log(LOG_GLOBAL_BUFFER);
    }
}


void run_simple_at_command_and_get_output(const char *command, int len, char *usefulOutput, int maxBufferLimit,
                                          const char *delimiter, unsigned char showCommandOutput, unsigned char strip)
{
    do_run_simple_at_command_and_get_output_with_timeout(command, len, usefulOutput, maxBufferLimit, delimiter, showCommandOutput, strip, 10);
}


void run_simple_at_command_and_get_output_with_user_timeout(const char *command, int len, char *usefulOutput, int maxBufferLimit,
                                                            const char *delimiter, unsigned char showCommandOutput, unsigned char strip,
                                                            int timeout)
{
    do_run_simple_at_command_and_get_output_with_timeout(command, len, usefulOutput, maxBufferLimit, delimiter, showCommandOutput, strip,
                                                         timeout);
}

#endif
