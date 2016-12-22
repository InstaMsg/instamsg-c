#include "../../instamsg/driver/include/instamsg.h"
#include "../utils/publisher_subscriber_init.h"

char TOPIC[100];


void release_app_resources()
{
}


static int oneToOneResponseReceivedCallback(OneToOneResult* result)
{
    sg_sprintf(LOG_GLOBAL_BUFFER, "Received [%s] from peer [%s]", result->peerMsg, result->peerClientId);
    info_log(LOG_GLOBAL_BUFFER);

    return SUCCESS;
}


static int onConnectOneTimeOperations()
{
    static unsigned char onceDone = 0;
    if(onceDone == 1)
    {
        return SUCCESS;
    }


    if(instaMsg.connected == 1)
    {
        onceDone = 1;

        if(strlen(TOPIC) == 0)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, "Peer Client-Id not specified .. exiting");
            error_log(LOG_GLOBAL_BUFFER);

            exitApp();
        }

        sendOneToOne(TOPIC,
                     "Hi... this is one-to-one initiator !!",
                     oneToOneResponseReceivedCallback,
                     3600);
    }
    else
    {
    }

    return SUCCESS;
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
    /*
     * Set the name of peer client-id here.
     */
    strcpy(TOPIC, "a14de8b0-c808-11e6-9650-bc764e106405");


    globalSystemInit(logFilePath);
    start(onConnectOneTimeOperations, NULL, NULL, NULL, 1);

    return 0;
}
