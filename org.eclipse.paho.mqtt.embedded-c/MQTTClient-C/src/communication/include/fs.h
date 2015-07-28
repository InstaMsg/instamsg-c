#ifndef INSTAMSG_FS
#define INSTAMSG_FS

#include "./common.h"

typedef struct FileSystem FileSystem;
struct FileSystem
{
    COMMUNICATION_FIELDS
};

FS* get_new_file_system();
void release_network(FileSystem*);

#endif
