#include "../../instamsg/driver/include/instamsg.h"

void release_app_resources()
{
}


int main(int argc, char** argv)
{
    char *logFilePath = NULL;

#if FILE_SYSTEM_ENABLED == 1
    globalSystemInit("./instamsg.log");
#else
    globalSystemInit(NULL);
#endif

    start(NULL, NULL, NULL, NULL, 300);
}
