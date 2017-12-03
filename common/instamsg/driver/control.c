/*******************************************************************************
 *
 * Copyright (c) 2014 SenseGrow, Inc.
 *
 * SenseGrow Internet of Things (IoT) Client Frameworks
 * http://www.sensegrow.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.

 * Contributors:
 *    Ajay Garg <ajay.garg@sensegrow.com>
 *******************************************************************************/

#include <string.h>

#include "./include/globals.h"
#include "./include/control.h"
#include "./include/time.h"
#include "./include/json.h"
#include "./include/log.h"
#include "./include/sg_stdlib.h"
#include "./include/gpio.h"

#if MODBUS_INTERFACE_ENABLED == 1
#include "../../ioeye/include/serial.h"
#endif

static char temp[20];
static int isOkToRunControlCommandTimeWise(char *outerJson)
{
    unsigned long utcTimeStampFromControlCommandLong = 0;
    unsigned long currentUTCTimeStamp = 0;

    int ttl = 0;

    memset(temp, 0, sizeof(temp));
    getJsonKeyValueIfPresent(outerJson, "time", temp);
    utcTimeStampFromControlCommandLong = sg_atoul(temp);

    memset(temp, 0, sizeof(temp));
    getJsonKeyValueIfPresent(outerJson, "ttl", temp);
    ttl = sg_atoi(temp);

    currentUTCTimeStamp = getUTCTimeStamp();
    if(currentUTCTimeStamp < ( utcTimeStampFromControlCommandLong + ((unsigned long)ttl) ) )
    {
        return SUCCESS;
    }
    else
    {
        return FAILURE;
    }
}


static char outerJson[200];
static char innerJson[200];
struct ControlCommandParams controlCommandParams;

void processControlCommand(char *controlCommandPayload)
{
    sg_sprintf(LOG_GLOBAL_BUFFER, "Control-Command ==> [%s]", controlCommandPayload);
    info_log(LOG_GLOBAL_BUFFER);

    memset(innerJson, 0, sizeof(innerJson));
    memset(outerJson, 0, sizeof(outerJson));

    memset(controlCommandParams.portName, 0, sizeof(controlCommandParams.portName));
    memset(controlCommandParams.portAddress, 0, sizeof(controlCommandParams.portAddress));
    memset(controlCommandParams.hostAddress, 0, sizeof(controlCommandParams.hostAddress));
    memset(controlCommandParams.hostPort, 0, sizeof(controlCommandParams.hostPort));
    memset(controlCommandParams.command, 0, sizeof(controlCommandParams.command));

    if(strstr(controlCommandPayload, "\"v\"") != NULL)
    {
        /*
         * The string received from the server contains the port-information
         */
        get_inner_outer_json_from_two_level_json(controlCommandPayload, "\"port\"", outerJson, sizeof(outerJson), innerJson, sizeof(innerJson));

        sg_sprintf(LOG_GLOBAL_BUFFER, "Control-Command-Inner-Json ==> [%s]", innerJson);
        info_log(LOG_GLOBAL_BUFFER);

        sg_sprintf(LOG_GLOBAL_BUFFER, "Control-Command-Outer-Json ==> [%s]", outerJson);
        info_log(LOG_GLOBAL_BUFFER);

        if(isOkToRunControlCommandTimeWise(outerJson) == FAILURE)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, "Time has expired to run the control-command [%s], not doing anything ...", controlCommandPayload);
            error_log(LOG_GLOBAL_BUFFER);

            return;
        }


        /*
         * Get the command to run.
         */
        getJsonKeyValueIfPresent(outerJson, "data", controlCommandParams.command);

        sg_sprintf(LOG_GLOBAL_BUFFER, "Actual command parsed from control-action command [%s] is [%s]",
                                      controlCommandPayload, controlCommandParams.command);
        info_log(LOG_GLOBAL_BUFFER);


        /*
         * Get the port-information.
         */
        getJsonKeyValueIfPresent(innerJson, "port_name", controlCommandParams.portName);
        getJsonKeyValueIfPresent(innerJson, "port_address", controlCommandParams.portAddress);
        getJsonKeyValueIfPresent(innerJson, "host_address", controlCommandParams.hostAddress);
        getJsonKeyValueIfPresent(innerJson, "host_port", controlCommandParams.hostPort);

    }
    else
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "No-version information in control-action command [%s]", controlCommandPayload);
        error_log(LOG_GLOBAL_BUFFER);

        strcpy(controlCommandParams.command, controlCommandPayload);
    }

    if(strcmp(controlCommandParams.portName, PORT_NAME_GPIO) == 0)
    {
        performDigitalOutputAction(controlCommandParams.command);
    }
    else
    {
#if MODBUS_INTERFACE_ENABLED == 1
        /*
         * It's a modbus-command, over a COM-port.
         */
        int i = 0;
        for(i = 0; i < MAX_PORTS_ALLOWED; i++)
        {
            Serial *s = &(serialInterfaces[i]);
            if( (strcmp(s->portName, PORT_NAME_COM) == 0) && (sg_atoi(s->portAddress) == sg_atoi(controlCommandParams.portAddress)) )
            {
                processCommand(controlCommandParams.command, s);
            }
        }
#endif
    }
}
