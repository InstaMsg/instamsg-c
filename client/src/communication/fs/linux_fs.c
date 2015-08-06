/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/



#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "instamsg_vendor.h"
#include "../../common/include/globals.h"



static void release_underlying_medium_guaranteed(FileSystem* fs)
{
    FILE *fp = fs->fp;
    if(fp != NULL)
    {
        fclose(fp);
        fp = NULL;      // This helps to keep track if a file is closed already.
    }
}


static void connect_underlying_medium_guaranteed(FileSystem* fs, const char *fileName)
{
    fs->fp = fopen(fileName, "a+");
}


static int linux_fs_read(FileSystem* fs, unsigned char* buffer, int len)
{
    if(fread(buffer, sizeof(buffer[0]), len, fs->fp) < len)
    {
        return FAILURE;
    }

    return SUCCESS;
}


static int linux_fs_write(FileSystem* fs, unsigned char* buffer, int len)
{
    if(fwrite(buffer, sizeof(buffer[0]), len, fs->fp) < len)
    {
        return FAILURE;
    }
    fflush(fs->fp);

    return SUCCESS;
}


void init_file_system(FileSystem *fs, void *arg)
{
    // Register read-callback.
	fs->read = linux_fs_read;

    // Register write-callback.
	fs->write = linux_fs_write;

    // Connect the medium (file-pointer).
    const char *fileName = (const char*)arg;
    connect_underlying_medium_guaranteed(fs, fileName);
}


int rename_file_system(const char *oldPath, const char *newPath)
{
    return rename(oldPath, newPath);
}


int delete_file_system(const char *filePath)
{
    return remove(filePath);
}


void release_file_system(FileSystem *fs)
{
    release_underlying_medium_guaranteed(fs);
}
