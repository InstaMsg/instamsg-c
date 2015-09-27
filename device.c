#include "./instamsg/driver/include/instamsg.h"
#include "./instamsg/driver/include/globals.h"

#include <string.h>

static void publishAckReceived(MQTTFixedHeaderPlusMsgId *fixedHeaderPlusMsgId)
{
    info_log("PUBACK received for msg-id [%u]", fixedHeaderPlusMsgId->msgId);
}


static char msgBuf[10];
const char *topic = "listener_topic";

void coreLoopyBusinessLogicInitiatedBySelf()
{
    int rc;

    startAndCountdownTimer(3, 0);
	if(1)
	{
        memset(msgBuf, 0, 10);

        static long counter;
        counter++;
        sg_sprintf(msgBuf, "Test %u", counter);

		info_log("Publishing message [%s] to %s", msgBuf, topic);
		rc = MQTTPublish(&instaMsg, topic, (const char*)msgBuf, 2, 0,
                         publishAckReceived, MQTT_RESULT_HANDLER_TIMEOUT, 0);
		info_log("Published %d", rc);

#ifdef DEBUG_MODE
        startAndCountdownTimer(300, 1);
#endif
	}
}


int main(int argc, char** argv)
{
    globalSystemInit();
    start(NULL, NULL, NULL, coreLoopyBusinessLogicInitiatedBySelf, NULL);
}

