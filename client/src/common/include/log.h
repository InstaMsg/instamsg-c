#ifndef INSTAMSG_LOGGER
#define INSTAMSG_LOGGER

#include "../../communication/include/fs.h"

typedef struct Logger Logger;
struct Logger
{
    FileSystem *medium;
};

Logger* get_new_logger(void *arg);
void release_logger(Logger*);

#endif
