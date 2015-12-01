#include "./include/modbus.h"
#include "./include/globals.h"
#include "./include/hex.h"
#include "./include/time.h"
#include "./include/data_logger.h"

#include "../../instamsg/driver/include/log.h"
#include "../../instamsg/driver/include/instamsg.h"
#include "../../instamsg/driver/include/sg_mem.h"
#include "../../instamsg/driver/include/watchdog.h"
#include "../../instamsg/driver/include/misc.h"
#include "../../instamsg/driver/include/config.h"
#include "../../instamsg/driver/include/globals.h"

#include <string.h>

static char watchdogAssistant[50];
static char smallBuffer[MAX_BUFFER_SIZE / 2];


static int publishMessage(const char *topicName, char *message)
{
    return MQTTPublish(topicName,
                       message,
                       QOS1,
                       0,
                       NULL,
                       MQTT_RESULT_HANDLER_TIMEOUT,
                       0,
                       1);
}


static void addXMLFieldsInPayload(char *messageBuffer,
                                  char *tag,
                                  void (*func)(char *messageBuffer, int maxBufferLength))
{
    memset(watchdogAssistant, 0, sizeof(watchdogAssistant));
    strcat(watchdogAssistant, "Calculating-For-Payload ");
    strcat(watchdogAssistant, tag);

    strcat(messageBuffer, "<");
    strcat(messageBuffer, tag);
    strcat(messageBuffer, ">");

    memset(smallBuffer, 0, sizeof(smallBuffer));

    watchdog_reset_and_enable(10, watchdogAssistant);
    func(smallBuffer, sizeof(smallBuffer));
    watchdog_disable();

    strcat(messageBuffer, smallBuffer);

    strcat(messageBuffer, "</");
    strcat(messageBuffer, tag);
    strcat(messageBuffer, ">");
}


static void send_previously_unsent_modbus_data()
{
    /*
     * Also, try sending the records stored in the persistent-storage (if any).
     */
    while(1)
    {
        int rc;

        memset(messageBuffer, 0, sizeof(messageBuffer));
        rc = get_next_record_from_persistent_storage(messageBuffer, sizeof(messageBuffer));

        if(rc == SUCCESS)
        {
            /*
             * We got the record.
             */
            sg_sprintf(LOG_GLOBAL_BUFFER, "Sending data that could not be sent sometime earlier");
            info_log(LOG_GLOBAL_BUFFER);

            rc = publishMessage(TOPIC_WEBHOOK, messageBuffer);
            if(rc != SUCCESS)
            {
                sg_sprintf(LOG_GLOBAL_BUFFER,
                          "Since the data could not be sent to InstaMsg-Server, so not retrying sending data from persistent-storage");
                error_log(LOG_GLOBAL_BUFFER);

                break;
            }
        }
        else
        {
            /*
             * We did not get any record.
             */
            sg_sprintf(LOG_GLOBAL_BUFFER, "\n\nNo more pending-data to be sent from persistent-storage\n\n");
            info_log(LOG_GLOBAL_BUFFER);

            break;
        }
    }
}


/*
 * Function-Codes being handled ::
 *
 *  FC 1:   11 01 0013 0025 0E84
 *  FC 2:   11 02 00C4 0016 BAA9
 *  FC 3:   11 02 00C4 0016 BAA9
 *  FC 4:   11 04 0008 0001 B298
 */
static int validationCheck(char *commandNibbles)
{
    int i;
    int rc = FAILURE;

    unsigned char functionCodes[] = {
                                        '1',
                                        '2',
                                        '3',
                                        '4'
                                    };

    int commandNibblesLength = strlen(commandNibbles);
    if(commandNibblesLength < 12)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, MODBUS_ERROR "Modbus-Command Length less than 12");
        error_log(LOG_GLOBAL_BUFFER);

        return FAILURE;
    }

    /*
     * If the second byte (3rd and 4th nibbles) is not equal to 03 (reading-analog-registets), return -1
     */
    if((commandNibbles[2] != '0'))
    {
        goto exit;
    }

    for(i = 0; i < sizeof(functionCodes); i++)
    {
        if(commandNibbles[3] == functionCodes[i])
        {
            rc = SUCCESS;
            break;
        }
    }

exit:
    if(rc == FAILURE)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, MODBUS_ERROR "Modbus-Command-Code [%c%c] not one of 01 02 03 04 in command [%s]",
                               commandNibbles[2], commandNibbles[3], commandNibbles);
        error_log(LOG_GLOBAL_BUFFER);
    }

    return rc;
}


static unsigned long getExpectedModbusResponseLength(char *commandNibbles)
{
    int rc;
    unsigned long i = 0;

    rc = validationCheck(commandNibbles);
    if(rc != SUCCESS)
    {
        return rc;
    }

    /*
     * The 9, 10, 11, 12 nibbles contain the number of registers to be read.
     */
    i = i + (16 * 16 * 16 * getIntValueOfHexChar(commandNibbles[8]));
    i = i + (16 * 16 *      getIntValueOfHexChar(commandNibbles[9]));
    i = i + (16 *           getIntValueOfHexChar(commandNibbles[10]));
    i = i + (               getIntValueOfHexChar(commandNibbles[11]));

    i = i * 2;      /* 2 bytes per register */

    i = i + 3;      /* Id, Code, Bytes-Counter-Byte in the beginning*/
    i = i + 2;      /* 2 bytes for CRC in the end */

    return i;
}


static void fillPrefixIndices(char *commandNibbles, int *prefixStartIndex, int *prefixEndIndex)
{
    int rc = validationCheck(commandNibbles);
    if(rc != SUCCESS)
    {
        *prefixStartIndex = *prefixEndIndex = -1;
    }

    *prefixStartIndex = 4;
    *prefixEndIndex = 7;
}


static void fillModbusCommandResponseIntoMessageBuffer(char *messageBuffer, char *commandHexString, Modbus *modbus)
{
    int rc;
    unsigned char *responseByteBuffer = NULL;

    {
        int prefixStartIndex, prefixEndIndex, i;

        unsigned long responseLength = getExpectedModbusResponseLength(commandHexString);
        if(responseLength == FAILURE)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER,
                      "Some problem occurred while processing modbus-command [%s]. Not continuing in this cycle", commandHexString);
            error_log(LOG_GLOBAL_BUFFER);

            goto exit;
        }

        fillPrefixIndices(commandHexString, &prefixStartIndex, &prefixEndIndex);

        responseByteBuffer = (unsigned char*) sg_malloc(responseLength);
        if(responseByteBuffer == NULL)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, "Could not allocate memory for modbus-response-buffer :(");
            error_log(LOG_GLOBAL_BUFFER);

            goto exit;
        }

        sg_sprintf(LOG_GLOBAL_BUFFER, "Processing modbus-command [%s]", commandHexString);
        info_log(LOG_GLOBAL_BUFFER);

        RESET_GLOBAL_BUFFER;
        getByteStreamFromHexString(commandHexString, GLOBAL_BUFFER);

        sg_sprintf(LOG_GLOBAL_BUFFER, "Sending modbus-command [%s], and expecting response of [%u] bytes", commandHexString, responseLength);
        debug_log(LOG_GLOBAL_BUFFER);

        watchdog_reset_and_enable(10, "Getting-MODBUS-Response");
        rc = modbus->send_command_and_read_response_sync(modbus,
                                                         GLOBAL_BUFFER,
                                                         strlen(commandHexString) / 2,
                                                         responseByteBuffer,
                                                         responseLength);
        watchdog_disable();

        if(rc != SUCCESS)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, "Problem occured while fetching modbus-response... not proceeding further");
            error_log(LOG_GLOBAL_BUFFER);

            goto exit;
        }


        /*
         * Now, start filling the final modbus-response.
         */
        RESET_GLOBAL_BUFFER;


        /*
         * Fill in the prefix;
         */
        for(i = prefixStartIndex; i <= prefixEndIndex; i++)
        {
            char byte[2] = {0};
            sg_sprintf(byte, "%c", commandHexString[i]);

            strcat((char*)GLOBAL_BUFFER, byte);
        }

        /*
         * Fill-in the modbus-response-nibbles.
         */
        for(i = 0; i < responseLength; i++)
        {
            char hex[3] = {0};
            sg_sprintf(hex, "%x", responseByteBuffer[i]);

            if(responseByteBuffer[i] <= 0x0F)
            {
                strcat((char*)GLOBAL_BUFFER, "0");
            }
            strcat((char*)GLOBAL_BUFFER, hex);
        }

        sg_sprintf(LOG_GLOBAL_BUFFER, "Modbus-Command [%s], Modbus-Response [%s]", commandHexString, (char*)GLOBAL_BUFFER);
        debug_log(LOG_GLOBAL_BUFFER);
        strcat(messageBuffer, (char*) GLOBAL_BUFFER);
    }

exit:
    if(responseByteBuffer)
        sg_free(responseByteBuffer);
}


static void processModbusCommand(char *commandHexString, Modbus *modbus)
{
    int rc;

    /*
     * Now, start forming the payload ....
     */
    memset(messageBuffer, 0, sizeof(messageBuffer));
    strcat(messageBuffer, "<rtu>");

    addXMLFieldsInPayload(messageBuffer, "manufacturer", get_manufacturer);


    /*
     * Modbus-Response
     */
    strcat(messageBuffer, "<data><![CDATA[");
    fillModbusCommandResponseIntoMessageBuffer(messageBuffer, commandHexString, modbus);
    strcat(messageBuffer, "]]></data>");


    addXMLFieldsInPayload(messageBuffer, "serial_number", get_device_uuid);
    addXMLFieldsInPayload(messageBuffer, "time", getTimeInDesiredFormat);
    addXMLFieldsInPayload(messageBuffer, "offset", getTimezoneOffset);

    strcat(messageBuffer, "</rtu>");

    sg_sprintf(LOG_GLOBAL_BUFFER, "Sending device-data [%s]", messageBuffer);
    debug_log(LOG_GLOBAL_BUFFER);

    /*
     * The net-section can be used for testing the Flash-Memory Storage/Retrieval as and when required.
     */
#if 0
    rc = FAILURE;
#else
    rc = publishMessage(TOPIC_WEBHOOK, messageBuffer);
#endif
    if(rc != SUCCESS)
    {
        /*
         * If the data could not be sent, we need to log it, so that it can be re-attempted (later).
         */
        save_record_to_persistent_storage(messageBuffer);
    }
}


















void init_modbus(Modbus *modbus, void *arg)
{
    static int interfaceNumber = 0;
    interfaceNumber++;

    modbus->interfaceNumber = interfaceNumber;

    /* Register the callback */
	modbus->send_command_and_read_response_sync = modbus_send_command_and_read_response_sync;

    /* Zero-initialize the buffer */
    memset(modbus->modbusCommands, 0, sizeof(modbus->modbusCommands));

    connect_underlying_modbus_medium_guaranteed(modbus);
}


void release_modbus(Modbus *modbus)
{
    release_underlying_modbus_medium_guaranteed(modbus);
}


void modbusOnConnectProcedures(Modbus *modbus)
{
    memset(smallBuffer, 0, sizeof(smallBuffer));
    sg_sprintf(smallBuffer, "MODBUS_COMMANDS_INTERFACE_%u", modbus->interfaceNumber);

    registerEditableConfig(modbus->modbusCommands,
                           smallBuffer,
                           CONFIG_STRING,
                           "",
                           "Comma-Separated List of Hexified-Modbus-Commands");

    send_previously_unsent_modbus_data();
}


void modbusProcedures(Modbus *modbus)
{
    char *saveptr;
    char *command;

    char *temporaryCopy = NULL;

    if(strlen(modbus->modbusCommands) > 0)
    {
        temporaryCopy = (char*) sg_malloc(sizeof(modbus->modbusCommands) + 1);
        if(temporaryCopy == NULL)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, MODBUS_ERROR "Could not allocate temporary-memory for tokenizing modbus-commands");
            error_log(LOG_GLOBAL_BUFFER);

            goto exit;
        }

        strcpy(temporaryCopy, modbus->modbusCommands);

        command = strtok_r(temporaryCopy, ",", &saveptr);
        while(command != NULL)
        {
            processModbusCommand(command, modbus);
            command = strtok_r(NULL, ",", &saveptr);
        }
    }
    else
    {
        sg_sprintf(LOG_GLOBAL_BUFFER,
                   MODBUS_ERROR "No modbus-commands to execute !!!; please fill-in some commands on the InstaMsg-Server for this device");
        info_log(LOG_GLOBAL_BUFFER);
    }

exit:
    if(temporaryCopy)
        sg_free(temporaryCopy);
}
