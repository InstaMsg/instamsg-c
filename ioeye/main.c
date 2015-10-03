#include "../instamsg/driver/include/instamsg.h"
#include "../instamsg/driver/include/globals.h"
#include "../instamsg/driver/include/sg_mem.h"
#include "../instamsg/driver/include/watchdog.h"

#include "./include/globals.h"
#include "./include/metadata.h"
#include "./include/modbus.h"
#include "./include/hex.h"
#include "./include/time.h"

#include "device_modbus.h"

static char messageBuffer[MAX_BUFFER_SIZE];
static char smallBuffer[MAX_BUFFER_SIZE / 2];
static char watchdogAssistant[50];

Modbus singletonModbusInterface;

static void sendClientData(void (*func)(char *messageBuffer, int maxBufferLength),
                          const char *topicName)
{
    /*
     * This method sends the data upon client's connect.
     *
     * If the message(s) are not sent from this method, that means that the connection is not (fully) completed.
     * Thus, the InstaMsg-Driver code will try again for the connection, and then these messages will be sent (again).
     *
     * Bottom-line : We do not need to re-attempt the message(s) sent by this method.
     */

    memset(messageBuffer, 0, sizeof(messageBuffer));
    func(messageBuffer, sizeof(messageBuffer));

    MQTTPublish(&instaMsg,
                topicName,
                messageBuffer,
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

    sendClientData(get_client_session_data, TOPIC_SESSION_DATA);
    sendClientData(get_client_metadata, TOPIC_METADATA);
    sendClientData(get_network_data, TOPIC_NETWORK_DATA);

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


static void coreLoopyBusinessLogicInitiatedBySelf()
{
    int rc;
    int i;
    unsigned char *responseByteBuffer;


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
        char *commandHexString = "03030064000A85F0";

        unsigned long responseLength = getExpectedModbusResponseLength(commandHexString);

        responseByteBuffer = (unsigned char*) sg_malloc(responseLength);
        if(responseByteBuffer == NULL)
        {
            error_log("Could not allocate memory for modbus-response-buffer :(");
            goto exit;
        }

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


    addXMLFieldsInPayload(messageBuffer, "serial_number", get_serial_number);
    addXMLFieldsInPayload(messageBuffer, "time", getTimeInDesiredFormat);
    addXMLFieldsInPayload(messageBuffer, "offset", getTimezoneOffset);

    strcat(messageBuffer, "</rtu>");

    debug_log("Sending device-data [%s]", messageBuffer);
    rc = MQTTPublish(&instaMsg,
                     TOPIC_WEBHOOK,
                     messageBuffer,
                     QOS1,
                     0,
                     NULL,
                     MQTT_RESULT_HANDLER_TIMEOUT,
                     0,
                     1);

    if(rc != SUCCESS)
    {
    }


exit:
    if(responseByteBuffer)
        sg_free(responseByteBuffer);

    startAndCountdownTimer(300, 0);
}


int main(int argc, char** argv)
{
#ifdef FILE_SYSTEM_INTERFACE_ENABLED
    globalSystemInit(LOG_FILE_PATH);
#else
    globalSystemInit(NULL);
#endif

    start(onConnect, NULL, NULL, coreLoopyBusinessLogicInitiatedBySelf);
}
