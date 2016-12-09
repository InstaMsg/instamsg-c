#include "./include/at.h"
#include "./include/watchdog.h"
#include "./include/sg_stdlib.h"

#include <string.h>

unsigned char showCommandOutput;

void run_simple_at_command_and_get_output(const char *command, char *usefulOutput, int maxBufferLimit, const char *delimiter)
{
    unsigned char watchdog_enable_required = 0;

    memset(usefulOutput, 0, maxBufferLimit);

    if(watchdog_active == 0)
    {
        watchdog_enable_required = 1;
        watchdog_reset_and_enable(10, (char*)command, 1);
    }

    do_fire_at_command_and_get_output(command, usefulOutput, delimiter);
    strip_leading_and_trailing_white_paces(usefulOutput);

    if(watchdog_enable_required == 1)
    {
        watchdog_disable(NULL, NULL);
    }
}
