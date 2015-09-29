#ifndef IOEYE_METADATA
#define IOEYE_METADATA

#include <string.h>

#include "../../../../instamsg/driver/include/globals.h"
#include "../../../../instamsg/driver/include/log.h"
#include "../../../../instamsg/driver/include/watchdog.h"
#include "../../../../instamsg/driver/include/sg_stdlib.h"

#include "../common/telit.h"

static void addKeyValue(char *buffer, const char *key, const char *splitter, const char *defaultValue, const char *command,
                        unsigned char addComma)
{
    if(addComma == 1)
    {
        strcat(buffer, ", ");
    }

    strcat(buffer, "'");
    strcat(buffer, key);
    strcat(buffer, "' : '");

    RESET_GLOBAL_BUFFER;
    run_simple_at_command_and_get_output(command, (char*)GLOBAL_BUFFER);

    if(splitter != NULL)
    {
        /*
         * Attach the splitted-value.
         */
        char *starter = strstr((char*)GLOBAL_BUFFER, splitter);
        if(starter != NULL)
        {
            /*
             * Ignore the starting double-quote.
             */
            if(starter[1] == '"')
            {
                starter = starter + 1;
            }

            strcat(buffer, starter + 1);

            /*
             * Ignore the ending double-quote.
             */
            if(buffer[strlen(buffer) - 1] == '"')
            {
                buffer[strlen(buffer) - 1] = 0;
            }
        }
        else
        {
            if(defaultValue != NULL)
            {
                strcat(buffer, defaultValue);
            }
        }
    }
    else
    {
        /*
         * Attach direct, complete value.
         */
        strcat(buffer, (char*)GLOBAL_BUFFER);
    }

    strcat(buffer, "'");
}


static void addSignalStrength(char *buffer, unsigned char addComma)
{
    if(addComma == 1)
    {
        strcat(buffer, ", ");
    }

    strcat(buffer, "'signal_strength' : '");

    RESET_GLOBAL_BUFFER;
    run_simple_at_command_and_get_output("AT+CSQ\r\n", (char*)GLOBAL_BUFFER);
    info_log("Signal-Strength AT-Command Raw-Output = [%s]", GLOBAL_BUFFER);

    /*
     * At this point, we have the output as "+CSQ: 15,0"
     */
    char *firstValueStart = (char*)GLOBAL_BUFFER + strlen("+CSQ: ");
    char *finder = strstr(firstValueStart, ",");

    {
        int intValue = -1;

        char intString[6] = {0};
        memcpy(intString, firstValueStart, finder - firstValueStart);

        intValue = sg_atoi(intString);
        intValue = -113 + (2 * intValue);

        memset(intString, 0, sizeof(intString));
        sg_sprintf(intString, "%d", intValue);

        strcat(buffer, intString);
    }

    strcat(buffer, "'");
}


/*
 * This method returns the client-session-data, in simple JSON form, of type ::
 *
 * {key1 : value1, key2 : value2 .....}
 */
void get_client_session_data(char *messageBuffer, int maxBufferLength)
{
    watchdog_reset_and_enable(60, "get_client_session_data");


    /*
     * Start the JSON-Dict.
     */
    strcat(messageBuffer, "{");

    strcat(messageBuffer, "'method' : 'GPRS'");

    addKeyValue(messageBuffer, "ip_address", ",", "\"\"", "AT+CGPADDR=\r\n", 1);
    addKeyValue(messageBuffer, "antina_status", ":", " -1", "AT#GSMAD=3\r\n", 1);

    addSignalStrength(messageBuffer, 1);

    /*
     * Terminate the JSON-Dict.
     */
    strcat(messageBuffer, "}");

    info_log("Client-Session-Data = [%s]", messageBuffer);
    watchdog_disable();
}


/*
 * This method returns the client-metadata, in simple JSON form, of type ::
 *
 * {key1 : value1, key2 : value2 .....}
 */
void get_client_metadata(char *messageBuffer, int maxBufferLength)
{
    watchdog_reset_and_enable(60, "get_client_metadata");


    /*
     * Start the JSON-Dict.
     */
    strcat(messageBuffer, "{");

    addKeyValue(messageBuffer, "imei", NULL, NULL, "AT+CGSN\r\n", 0);
    addKeyValue(messageBuffer, "serial_number", NULL, NULL, "AT+CGSN\r\n", 1);
    addKeyValue(messageBuffer, "model", NULL, NULL, "AT+GMM\r\n", 1);
    addKeyValue(messageBuffer, "firmware_version", NULL, NULL, "AT+GMR\r\n", 1);
    addKeyValue(messageBuffer, "manufacturer", NULL, NULL, "AT+GMI\r\n", 1);

    strcat(messageBuffer, ", 'client_version' : '");
    strcat(messageBuffer, INSTAMSG_VERSION);
    strcat(messageBuffer, "'");


    /*
     * Terminate the JSON-Dict.
     */
    strcat(messageBuffer, "}");

    info_log("Client-Metadata = [%s]", messageBuffer);
    watchdog_disable();
}


/*
 * This method returns the client-network-data, in simple JSON form, of type ::
 *
 * {key1 : value1, key2 : value2 .....}
 */
void get_network_data(char *messageBuffer, int maxBufferLength)
{
    watchdog_reset_and_enable(60, "get_network_data");

    /*
     * Start the JSON-Dict.
     */
    strcat(messageBuffer, "{");

    addKeyValue(messageBuffer, "antina_status", ":", " -1", "AT#GSMAD=3\r\n", 0);
    addSignalStrength(messageBuffer, 1);

    /*
     * Terminate the JSON-Dict.
     */
    strcat(messageBuffer, "}");

    info_log("Client-Network-Data = [%s]", messageBuffer);
    watchdog_disable();
}


#endif
