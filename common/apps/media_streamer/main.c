#include "../../instamsg/driver/include/instamsg.h"

static void coreLoopyBusinessLogicInitiatedBySelf()
{
    static unsigned char streamingStarted = 0;

    if(streamingStarted == 0)
    {
        initiate_streaming();
        streamingStarted = 1;
    }
}


int main(int argc, char** argv)
{
    char *logFilePath = NULL;

#ifdef FILE_SYSTEM_INTERFACE_ENABLED
    logFilePath = LOG_FILE_PATH;
#else
    logFilePath = NULL;
#endif

    globalSystemInit(logFilePath);
    start(NULL, NULL, NULL, coreLoopyBusinessLogicInitiatedBySelf, 3);
}
