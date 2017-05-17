#include "../../../common/ioeye/include/serial.h"

#include "../../../common/instamsg/driver/include/log.h"
#include "../../../common/instamsg/driver/include/sg_stdlib.h"


void processControlCommand(const char *command)
{
    char actualCommand[50] = {0};

    /*
     * The string received from server is of the form ::
     *
     * /com/0/command
     */
    get_nth_token_thread_safe((char*) command, '/', 4, actualCommand, 1);
    sg_sprintf(LOG_GLOBAL_BUFFER, "Actual command parsed from control-action command [%s] is [%s]", command, actualCommand);
    info_log(LOG_GLOBAL_BUFFER);

    processCommand(actualCommand, &(serialInterfaces[0]));
}
