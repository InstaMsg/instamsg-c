#include "../../instamsg/driver/include/instamsg.h"
#include "../../instamsg/driver/include/sg_mem.h"
#include "../utils/publisher_subscriber_init.h"

char TOPIC[100];

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
