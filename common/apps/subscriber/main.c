#include "../../instamsg/driver/include/instamsg.h"
#include "../../instamsg/driver/include/sg_mem.h"

char TOPIC[100];

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
    return subscribe(TOPIC,
                     QOS2,
                     messageArrived,
                     subscribeAckReceived,
                     MQTT_RESULT_HANDLER_TIMEOUT,
                     1);
}


void release_app_resources()
{
}


int main(int argc, char** argv)
{
    char *logFilePath = NULL;

#if FILE_SYSTEM_ENABLED == 1
    logFilePath = "instamsg.log";
#else
    logFilePath = NULL;
#endif

    strcpy(TOPIC, "listener_topic");

    globalSystemInit(logFilePath);
    start(onConnectOneTimeOperations, NULL, NULL, NULL, 1);
}
