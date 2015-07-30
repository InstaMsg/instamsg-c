/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/



#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "../include/fs.h"
#include "../../common/include/instamsg.h"
#include "../../../../MQTTPacket/src/common.h"



static int linux_fs_read(FileSystem* fs, unsigned char* buffer, int len);
static int linux_fs_write(FileSystem* fs, unsigned char* buffer, int len);


#define GET_IMPLEMENTATION_SPECIFIC_MEDIUM_OBJ(fs) ((FILE *)(fs->medium))


static void release_underlying_medium_guaranteed(FileSystem* fs)
{
    // Close the file-pointer.
    //
    FILE *fp = GET_IMPLEMENTATION_SPECIFIC_MEDIUM_OBJ(fs);
    if(fp != NULL)
    {
        fclose(fp);
    }
}


static void connect_underlying_medium_guaranteed(FileSystem* fs, const char *fileName)
{
    fs->medium = fopen(fileName, "a+");
}


static int linux_fs_read(FileSystem* fs, unsigned char* buffer, int len)
{
    if(fread(buffer, sizeof(buffer[0]), len, fs->medium) < len)
    {
        return FAILURE;
    }

    return SUCCESS;
}


static int linux_fs_write(FileSystem* fs, unsigned char* buffer, int len)
{
    if(fwrite(buffer, sizeof(buffer[0]), len, fs->medium) < len)
    {
        return FAILURE;
    }
    fflush(fs->medium);

    return SUCCESS;
}


FileSystem* get_new_file_system(void *arg)
{
    FileSystem *fs = (FileSystem*)malloc(sizeof(FileSystem));

    // Here, physical medium is a file-pointer.
	fs->medium = malloc(sizeof(FILE *));

    // Register read-callback.
	fs->read = linux_fs_read;

    // Register write-callback.
	fs->write = linux_fs_write;

    // Connect the medium (file-pointer).
    const char *fileName = (const char*)arg;
    connect_underlying_medium_guaranteed(fs, fileName);

    return fs;
}


void release_file_system(FileSystem *fs)
{
    release_underlying_medium_guaranteed(fs);

    // Free the dynamically-allocated memory
    if(fs != NULL)
    {
        free(fs);
    }
}
