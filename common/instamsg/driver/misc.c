#ifndef MISC_COMMON_IMPL
#define MISC_COMMON_IMPL

#include "./include/misc.h"
#include "./include/data_logger.h"
#include "./include/config.h"


void exitApp()
{
    release_app_resources();

    release_data_logger();
    release_config();

#if FILE_LOGGING_ENABLED == 1
    release_file_logger(&fileLogger);
#endif

    resetDevice();
}


#endif
