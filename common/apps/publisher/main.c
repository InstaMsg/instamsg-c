#include "../../instamsg/driver/include/instamsg.h"

static void publishAckReceived(MQTTFixedHeaderPlusMsgId *fixedHeaderPlusMsgId)
{
    info_log("PUBACK received for msg-id [%u]", fixedHeaderPlusMsgId->msgId);
}


static void coreLoopyBusinessLogicInitiatedBySelf()
{
    char buf[20] = {0};

    static int counter;
    counter++;
    sg_sprintf(buf, "Test %d", counter);

    MQTTPublish(&instaMsg,
                "listener_topic",
                buf,
                QOS2,
                0,
                publishAckReceived,
                MQTT_RESULT_HANDLER_TIMEOUT,
                0,
                1);
}


int main(int argc, char** argv)
{
#ifdef FILE_SYSTEM_INTERFACE_ENABLED
    globalSystemInit(LOG_FILE_PATH);
#else
    globalSystemInit(NULL);
#endif

    start(NULL, NULL, NULL, coreLoopyBusinessLogicInitiatedBySelf, 3);
}
