#include "../instamsg/driver/include/instamsg.h"
#include "../instamsg/driver/include/globals.h"

#include "./include/globals.h"
#include "./include/metadata.h"

static char messageBuffer[MAX_BUFFER_SIZE];

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
    sendClientData(get_client_session_data, TOPIC_SESSION_DATA);
    sendClientData(get_client_metadata, TOPIC_METADATA);
    sendClientData(get_network_data, TOPIC_NETWORK_DATA);

    return SUCCESS;
}


void coreLoopyBusinessLogicInitiatedBySelf()
{
    startAndCountdownTimer(300, 1);
}


int main(int argc, char** argv)
{
    globalSystemInit();
    start(onConnect, NULL, NULL, coreLoopyBusinessLogicInitiatedBySelf, NULL);
}
