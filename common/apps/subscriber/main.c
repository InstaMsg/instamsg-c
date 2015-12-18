#include "../../instamsg/driver/include/instamsg.h"
#include "../../instamsg/driver/include/sg_mem.h"
#include "../utils/publisher_subscriber_init.h"

char TOPIC[100];

static int oneToOneResponseReceivedCallback(OneToOneResult* result)
{
    sg_sprintf(LOG_GLOBAL_BUFFER, "Received [%s] from peer [%s]", result->peerMsg, result->peerClientId);
    info_log(LOG_GLOBAL_BUFFER);

    return SUCCESS;
}


static int oneToOneMessageHandler(OneToOneResult* result)
{
    char *msg = (char*) sg_malloc(1000);
    memset(msg, 0, 1000);

    sg_sprintf(LOG_GLOBAL_BUFFER, "Received [%s] from peer [%s]", result->peerMsg, result->peerClientId);
    info_log(LOG_GLOBAL_BUFFER);

    if(msg == NULL)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "Could not allocate memory for message :(");
        error_log(LOG_GLOBAL_BUFFER);

        return FAILURE;
    }

    memset(msg, 0, 1000);
    sg_sprintf(msg, "Got your response ==> %s :)", result->peerMsg);

    result->reply(result,
                  msg,
                  oneToOneResponseReceivedCallback,
                  3600);
    return SUCCESS;
}

static void subscribeAckReceived(MQTTFixedHeaderPlusMsgId *fixedHeaderPlusMsgId)
{
    sg_sprintf(LOG_GLOBAL_BUFFER, "SUBACK received for msg-id [%u]", fixedHeaderPlusMsgId->msgId);
    info_log(LOG_GLOBAL_BUFFER);
}


static void messageArrived(MessageData* md)
{
	MQTTMessage* message = md->message;

    sg_sprintf(LOG_GLOBAL_BUFFER, "%s", (char*)message->payload);
    info_log(LOG_GLOBAL_BUFFER);
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
    char *logFilePath = NULL;

#ifdef FILE_SYSTEM_INTERFACE_ENABLED
    logFilePath = LOG_FILE_PATH;
#else
    logFilePath = NULL;
#endif

    /*
     * This method is only for the test publisher/subscriber apps.
     * For real-world apps, this method will not be needed.
     */
    init_publisher_subscriber_params(argc, argv, &logFilePath);

    globalSystemInit(logFilePath);
    start(onConnectOneTimeOperations, NULL, oneToOneMessageHandler, NULL, 1);
}
