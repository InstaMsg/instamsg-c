#include "../../instamsg/driver/include/instamsg.h"

static int onConnectCallback()
{
    initiateStreaming();
    return SUCCESS;
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
    start(onConnectCallback, NULL, NULL, NULL, 3);
}
