#include "../../instamsg/driver/include/instamsg.h"
#include "../../instamsg/driver/include/globals.h"
#include "../../instamsg/driver/include/sg_mem.h"
#include "../../instamsg/driver/include/watchdog.h"
#include "../../instamsg/driver/include/misc.h"

#include "./include/globals.h"
#include "./include/modbus.h"
#include "./include/hex.h"
#include "./include/time.h"
#include "./include/data_logger.h"

#include "device_modbus.h"

static char smallBuffer[MAX_BUFFER_SIZE / 2];
static char watchdogAssistant[50];

Modbus singletonModbusInterface;

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


static int onConnect()
{
    init_modbus(&singletonModbusInterface, NULL);

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
            info_log("Sending data that could not be sent sometime earlier");

            rc = publishMessage(TOPIC_WEBHOOK, messageBuffer);
            if(rc != SUCCESS)
            {
                error_log("Since the data could not be sent to InstaMsg-Server, so not retrying sending data from persistent-storage");
                break;
            }
        }
        else
        {
            /*
             * We did not get any record.
             */
            info_log("\n\nNo more pending-data to be sent from persistent-storage\n\n");
            break;
        }
    }

    return SUCCESS;
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


static void processModbusCommand(char *commandHexString)
{
    int rc;
    int i;
    unsigned char *responseByteBuffer = NULL;

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

    {
        int prefixStartIndex, prefixEndIndex;

        unsigned long responseLength = getExpectedModbusResponseLength(commandHexString);
        if(responseLength == FAILURE)
        {
            error_log("Some problem occurred while processing modbus-command [%s]. Not continuing in this cycle", commandHexString);
            goto exit;
        }

        fillPrefixIndices(commandHexString, &prefixStartIndex, &prefixEndIndex);

        responseByteBuffer = (unsigned char*) sg_malloc(responseLength);
        if(responseByteBuffer == NULL)
        {
            error_log("Could not allocate memory for modbus-response-buffer :(");
            goto exit;
        }

        info_log("Processing modbus-command [%s]", commandHexString);
        RESET_GLOBAL_BUFFER;
        getByteStreamFromHexString(commandHexString, GLOBAL_BUFFER);

        debug_log("Sending modbus-command [%s], and expecting response of [%u] bytes", commandHexString, responseLength);

        watchdog_reset_and_enable(10, "Getting-MODBUS-Response");
        rc = singletonModbusInterface.send_command_and_read_response_sync(&singletonModbusInterface,
                                                                          GLOBAL_BUFFER,
                                                                          strlen(commandHexString) / 2,
                                                                          responseByteBuffer,
                                                                          responseLength);
        watchdog_disable();

        if(rc != SUCCESS)
        {
            error_log("Problem occured while fetching modbus-response... not proceeding further");
            goto exit;
        }


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

        debug_log("Modbus-Command [%s], Modbus-Response [%s]", commandHexString, (char*)GLOBAL_BUFFER);
        strcat(messageBuffer, (char*) GLOBAL_BUFFER);
    }

    strcat(messageBuffer, "]]></data>");


    addXMLFieldsInPayload(messageBuffer, "serial_number", get_device_uuid);
    addXMLFieldsInPayload(messageBuffer, "time", getTimeInDesiredFormat);
    addXMLFieldsInPayload(messageBuffer, "offset", getTimezoneOffset);

    strcat(messageBuffer, "</rtu>");

    debug_log("Sending device-data [%s]", messageBuffer);

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


exit:
    if(responseByteBuffer)
        sg_free(responseByteBuffer);

}


static void coreLoopyBusinessLogicInitiatedBySelf()
{
    char *modbusCommandsFromAPI = "03030064000A85F0,03050064000A85F0,03020064000A85F0";
    char *saveptr;
    char *command;

    char *modbusCommandStringMutable = (char*) sg_malloc(strlen(modbusCommandsFromAPI) + 1);
    if(modbusCommandStringMutable == NULL)
    {
        error_log(MODBUS_ERROR "Could not allocate memory for modbus-commands-conversion to mutable stream");
        goto exit;
    }
    strcpy(modbusCommandStringMutable, modbusCommandsFromAPI);

    command = strtok_r(modbusCommandStringMutable, ",", &saveptr);
    while(command != NULL)
    {
        processModbusCommand(command);
        command = strtok_r(NULL, ",", &saveptr);
    }

exit:
    if(modbusCommandStringMutable)
        sg_free(modbusCommandStringMutable);
}


int main(int argc, char** argv)
{
#ifdef FILE_SYSTEM_INTERFACE_ENABLED
    globalSystemInit(LOG_FILE_PATH);
#else
    globalSystemInit(NULL);
#endif
    init_data_logger_persistent_storage();

    start(onConnect, NULL, NULL, coreLoopyBusinessLogicInitiatedBySelf, 60);
}
