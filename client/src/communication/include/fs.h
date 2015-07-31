/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

/*
 * Interface for sending/receiving bytes between the instamg-client (on the device) and the device-file(system).
 */


#ifndef INSTAMSG_FS
#define INSTAMSG_FS

#include "./common.h"

typedef struct FileSystem FileSystem;
struct FileSystem
{
    COMMUNICATION_INTERFACE(FileSystem)
};

FileSystem* get_new_file_system(void *arg);
void release_file_system(FileSystem*);

#endif
