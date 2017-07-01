#include "../../../common/ioeye/include/serial.h"

#include "../../../common/instamsg/driver/include/log.h"
#include "../../../common/instamsg/driver/include/json.h"
#include "../../../common/instamsg/driver/include/sg_stdlib.h"


char outerJson[500];
char innerJson[500];
char actualCommand[50];

void processControlCommand(const char *command)
{
    sg_sprintf(LOG_GLOBAL_BUFFER, "Control-Command ==> [%s]", command);
    info_log(LOG_GLOBAL_BUFFER);

    memset(actualCommand, 0, sizeof(actualCommand));
    memset(innerJson, 0, sizeof(innerJson));
    memset(outerJson, 0, sizeof(outerJson));

    if(strstr(command, "\"v\"") != NULL)
    {
        /*
         * The string received from the server contains the port-information
         */
        get_inner_outer_json_from_two_level_json(command, "\"port\"", outerJson, sizeof(outerJson), innerJson, sizeof(innerJson));

        sg_sprintf(LOG_GLOBAL_BUFFER, "Control-Command-Inner-Json ==> [%s]", innerJson);
        info_log(LOG_GLOBAL_BUFFER);

        sg_sprintf(LOG_GLOBAL_BUFFER, "Control-Command-Outer-Json ==> [%s]", outerJson);
        info_log(LOG_GLOBAL_BUFFER);

        /*
         * Get the command to run.
         */
        getJsonKeyValueIfPresent(outerJson, "data", actualCommand);

        /*
         * Check if it is ok to run the command, expired-time wise
         *
         */
        {
            if(isOkToRunControlCommandTimeWise(outerJson) == FAILURE)
            {
                sg_sprintf(LOG_GLOBAL_BUFFER, "Time has expired to run the command [%s], not doing anything ...", actualCommand);
                error_log(LOG_GLOBAL_BUFFER);

                return;
            }
        }
    }
    else
    {
        /*
         * The string received from server is of the form ::
         *
         * /com/0/command
         */
        get_nth_token_thread_safe((char*) command, '/', 4, actualCommand, 1);

    }

    sg_sprintf(LOG_GLOBAL_BUFFER, "Actual command parsed from control-action command [%s] is [%s]", command, actualCommand);
    info_log(LOG_GLOBAL_BUFFER);

    processCommand(actualCommand, &(serialInterfaces[0]));
}
