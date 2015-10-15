#include "../../instamsg/driver/include/instamsg.h"

static char TOPIC[100];

static void oneToOneResponseReceivedCallback(OneToOneResult* result)
{
    info_log("Received [%s] from peer [%s]", result->peerMsg, result->peer);
}


static void onConnectOneTimeOperations()
{
    static unsigned char onceDone = 0;
    if(onceDone == 1)
    {
        return;
    }

    onceDone = 1;

    if(instaMsg.connected == 1)
    {
        MQTTSend(TOPIC,
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
    if(argc < 2)
    {
        return 1;
    }

    memset(TOPIC, 0, sizeof(TOPIC));
    strcpy(TOPIC, argv[1]);

    {
#ifdef FILE_SYSTEM_INTERFACE_ENABLED
        char *logFilePath = LOG_FILE_PATH;

#ifdef DEBUG_MODE

        if(argc >= 3)
        {
            memset(USER_LOG_FILE_PATH, 0, sizeof(USER_LOG_FILE_PATH));
            strcpy(USER_LOG_FILE_PATH, argv[2]);

            logFilePath = USER_LOG_FILE_PATH;
        }
        if(argc >= 4)
        {
            memset(USER_DEVICE_UUID, 0, sizeof(USER_DEVICE_UUID));
            strcpy(USER_DEVICE_UUID, argv[3]);
        }
#endif  /* DEBUG_MODE */

        globalSystemInit(logFilePath);

#else   /* FILE_SYSTEM_INTERFACE_ENABLED */

        globalSystemInit(NULL);

#endif  /* FILE_SYSTEM_INTERFACE_ENABLED */
    }

    start(NULL, NULL, NULL, onConnectOneTimeOperations, 1);
}
