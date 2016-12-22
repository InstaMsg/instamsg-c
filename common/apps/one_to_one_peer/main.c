#include "../../instamsg/driver/include/instamsg.h"
#include "../../instamsg/driver/include/sg_mem.h"

static int oneToOneResponseReceivedCallback(OneToOneResult* result)
{
    sg_sprintf(LOG_GLOBAL_BUFFER, "Received [%s] from peer [%s]", result->peerMsg, result->peerClientId);
    info_log(LOG_GLOBAL_BUFFER);

    return SUCCESS;
}


static int oneToOneMessageHandler(OneToOneResult* result)
{
    char *msg = (char*) sg_malloc(1000);
    memset(msg, 0, 1000);

    sg_sprintf(LOG_GLOBAL_BUFFER, "Received [%s] from peer [%s]", result->peerMsg, result->peerClientId);
    info_log(LOG_GLOBAL_BUFFER);

    if(msg == NULL)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "Could not allocate memory for message :(");
        error_log(LOG_GLOBAL_BUFFER);

        return FAILURE;
    }

    memset(msg, 0, 1000);
    sg_sprintf(msg, "Got your response ==> %s :)", result->peerMsg);

    result->reply(result,
                  msg,
                  oneToOneResponseReceivedCallback,
                  3600);

    sg_free(msg);
    return SUCCESS;
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

    globalSystemInit(logFilePath);
    start(NULL, NULL, oneToOneMessageHandler, NULL, 1);
}
