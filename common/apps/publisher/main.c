#include "../../instamsg/driver/include/instamsg.h"
#include "../utils/publisher_subscriber_init.h"

char TOPIC[100];

static void publishAckReceived(MQTTFixedHeaderPlusMsgId *fixedHeaderPlusMsgId)
{
    info_log("PUBACK received for msg-id [%u]", fixedHeaderPlusMsgId->msgId);
}


static void coreLoopyBusinessLogicInitiatedBySelf()
{
    char buf[20] = {0};

    static int counter = 0;
    counter++;
    sg_sprintf(buf, "Test %d", counter);

    MQTTPublish(TOPIC,
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
    start(NULL, NULL, NULL, coreLoopyBusinessLogicInitiatedBySelf, 3);
}
