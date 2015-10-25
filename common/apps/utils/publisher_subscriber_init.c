#include "../../instamsg/driver/include/instamsg.h"

extern char TOPIC[100];

void init_publisher_subscriber_params(int argc, char **argv, char **logFilePath)
{
    memset(TOPIC, 0, sizeof(TOPIC));
    strcpy(TOPIC, "listener_topic");

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

        *logFilePath = USER_LOG_FILE_PATH;
    }

    if(argc >= 4)
    {
        memset(USER_DEVICE_UUID, 0, sizeof(USER_DEVICE_UUID));
        strcpy(USER_DEVICE_UUID, argv[3]);
    }
#endif  /* DEBUG_MODE */
}

