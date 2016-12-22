#include "../../instamsg/driver/include/instamsg.h"
#include "../utils/publisher_subscriber_init.h"

char TOPIC[100];

static int oneToOneResponseReceivedCallback(OneToOneResult* result)
{
    sg_sprintf(LOG_GLOBAL_BUFFER, "Received [%s] from peer [%s]", result->peerMsg, result->peerClientId);
    info_log(LOG_GLOBAL_BUFFER);

    return SUCCESS;
}


static void onConnectOneTimeOperations()
{
    static unsigned char onceDone = 0;
    if(onceDone == 1)
    {
        return;
    }


    if(instaMsg.connected == 1)
    {
        onceDone = 1;

        send(TOPIC,
             "Hi... this is one-to-one initiator !!",
             oneToOneResponseReceivedCallback,
             3600);
    }
    else
    {
    }
}


int main(int argc, char** argv)
{
    char *logFilePath = NULL;

#if FILE_SYSTEM_ENABLED == 1
    logFilePath = "instamsg.log";
#else
    logFilePath = NULL;
#endif

    memset(TOPIC, 0, sizeof(TOPIC));
    strcpy(TOPIC, "listener_topic");


    globalSystemInit(logFilePath);
    start(NULL, NULL, NULL, coreLoopyBusinessLogicInitiatedBySelf, 1);

    return 0;
}
