#include "../instamsg/driver/include/instamsg.h"
#include "../instamsg/driver/include/globals.h"

#include "./include/globals.h"

static char messageBuffer[MAX_BUFFER_SIZE];
static int sendClientSessionData()
{
    int rc = FAILURE;

    memset(messageBuffer, 0, sizeof(messageBuffer));

    rc = MQTTPublish(&instaMsg,
                     TOPIC_SESSION_DATA,
                     messageBuffer,
                     QOS0,
                     0,
                     NULL,
                     MQTT_RESULT_HANDLER_TIMEOUT,
                     0,
                     1);

    return rc;
}


static int sendClientMetaData()
{
    return SUCCESS;
}


static int onConnect()
{
    int rc = FAILURE;

    rc = sendClientSessionData();
    rc = sendClientMetaData();

    return rc;
}


void coreLoopyBusinessLogicInitiatedBySelf()
{
    onConnect();
    startAndCountdownTimer(300, 1);
}


int main(int argc, char** argv)
{
    globalSystemInit();
    start(onConnect, NULL, NULL, coreLoopyBusinessLogicInitiatedBySelf, NULL);
}

