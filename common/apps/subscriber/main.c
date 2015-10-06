#include "../../instamsg/driver/include/instamsg.h"

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
    return MQTTSubscribe(&instaMsg,
                         "listener_topic",
                         QOS2,
                         messageArrived,
                         subscribeAckReceived,
                         MQTT_RESULT_HANDLER_TIMEOUT,
                         1);
}


static void coreLoopyBusinessLogicInitiatedBySelf()
{
}

int main(int argc, char** argv)
{
    start(onConnectOneTimeOperations, NULL, NULL, coreLoopyBusinessLogicInitiatedBySelf, 1);
}
