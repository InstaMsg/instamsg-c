#include "../../instamsg/driver/include/instamsg.h"

char TOPIC[100];




static void coreLoopyBusinessLogicInitiatedBySelf()
{
    char buf[20] = {0};

    static int counter = 0;
    counter++;
    sg_sprintf(buf, "Test %d", counter);

    publishMessageWithDeliveryGuarantee(TOPIC, buf);
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

    memset(TOPIC, 0, sizeof(TOPIC));
    strcpy(TOPIC, "listener_topic");


    globalSystemInit(logFilePath);
    start(NULL, NULL, NULL, coreLoopyBusinessLogicInitiatedBySelf, 3);

    return 0;
}
