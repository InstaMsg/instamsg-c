#include "../../instamsg/driver/include/instamsg.h"

static char TOPIC[100];

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

    start(NULL, NULL, NULL, coreLoopyBusinessLogicInitiatedBySelf, 3);
}
