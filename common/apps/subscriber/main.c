#include "../../instamsg/driver/include/instamsg.h"
#include "../../instamsg/driver/include/sg_mem.h"

static char TOPIC[100];

static int oneToOneMessageHandler(OneToOneResult* result)
{
    char *msg = (char*) sg_malloc(1000);

    info_log("Received [%s] from peer [%s]", result->peerMsg, result->peer);
    if(msg == NULL)
    {
        error_log("Could not allocate memory for message :(");
        return FAILURE;

    }

    memset(msg, 0, 1000);
    sg_sprintf(msg, "Got your response ==> %s :)", result->peerMsg);

    result->reply(result, msg);
    return SUCCESS;
}

static void subscribeAckReceived(MQTTFixedHeaderPlusMsgId *fixedHeaderPlusMsgId)
{
    info_log("SUBACK received for msg-id [%u]", fixedHeaderPlusMsgId->msgId);
}


static void messageArrived(MessageData* md)
{
	MQTTMessage* message = md->message;
    info_log("%s", (char*)message->payload);
}


static int onConnectOneTimeOperations()
{
    return MQTTSubscribe(TOPIC,
                         QOS2,
                         messageArrived,
                         subscribeAckReceived,
                         MQTT_RESULT_HANDLER_TIMEOUT,
                         1);
}


int main(int argc, char** argv)
{
    memset(TOPIC, 0, sizeof(TOPIC));
    strcpy(TOPIC, "listener_topic");

    {
#ifdef FILE_SYSTEM_INTERFACE_ENABLED
        char *logFilePath = LOG_FILE_PATH;

#ifdef DEBUG_MODE

        if(argc >= 2)
        {
            memset(TOPIC, 0, sizeof(TOPIC));
            strcpy(TOPIC, argv[1]);
        }

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

    start(onConnectOneTimeOperations, NULL, oneToOneMessageHandler, NULL, 1);
}
