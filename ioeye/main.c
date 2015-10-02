#include "../instamsg/driver/include/instamsg.h"
#include "../instamsg/driver/include/globals.h"
#include "../instamsg/driver/include/sg_mem.h"
#include "../instamsg/driver/include/watchdog.h"

#include "./include/globals.h"
#include "./include/metadata.h"
#include "./include/modbus.h"
#include "./include/hex.h"

#include "device_modbus.h"

static char messageBuffer[MAX_BUFFER_SIZE];
Modbus singletonModbusInterface;

static void sendClientData(void (*func)(char *messageBuffer, int maxBufferLength),
                          const char *topicName)
{
    int rc = FAILURE;

    memset(messageBuffer, 0, sizeof(messageBuffer));
    func(messageBuffer, sizeof(messageBuffer));

    rc = MQTTPublish(&instaMsg,
                     topicName,
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
}


static int onConnect()
{
    init_modbus(&singletonModbusInterface, NULL);

    sendClientData(get_client_session_data, TOPIC_SESSION_DATA);
    sendClientData(get_client_metadata, TOPIC_METADATA);
    sendClientData(get_network_data, TOPIC_NETWORK_DATA);

    return SUCCESS;
}


static void coreLoopyBusinessLogicInitiatedBySelf()
{
    int rc;

    unsigned char *responseByteBuffer;
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

    info_log("Sending modbus-command [%s], and expecting response of [%u] bytes", commandHexString, responseLength);

    watchdog_reset_and_enable(10, "send_command_and_read_response_sync");
    rc = singletonModbusInterface.send_command_and_read_response_sync(&singletonModbusInterface,
                                                                      GLOBAL_BUFFER,
                                                                      strlen(commandHexString) / 2,
                                                                      responseByteBuffer,
                                                                      responseLength);
    watchdog_disable();


    if(rc == SUCCESS)
    {
        int i;

        RESET_GLOBAL_BUFFER;
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

        info_log("Modbus-Command [%s], Modbus-Response [%s]", commandHexString, (char*)GLOBAL_BUFFER);
    }



exit:
    if(responseByteBuffer)
        sg_free(responseByteBuffer);

    startAndCountdownTimer(300, 1);
}


int main(int argc, char** argv)
{
    globalSystemInit();
    start(onConnect, NULL, NULL, coreLoopyBusinessLogicInitiatedBySelf, NULL);
}
