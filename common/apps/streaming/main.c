#include "../../instamsg/driver/include/instamsg.h"

int main(int argc, char** argv)
{
    char *logFilePath = NULL;

#if FILE_SYSTEM_ENABLED == 1
    logFilePath = LOG_FILE_PATH;
#else
    logFilePath = NULL;
#endif

    globalSystemInit(logFilePath);
    start(NULL, NULL, NULL, NULL, 300);
}
