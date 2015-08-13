/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/



#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>

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


static int linux_fs_read(FileSystem* fs, unsigned char* buffer, int len, unsigned char guaranteed)
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


static int renameFile(FileSystem *fs, const char *oldPath, const char *newPath)
{
    return rename(oldPath, newPath);
}


static int deleteFile(FileSystem *fs, const char *filePath)
{
    return remove(filePath);
}


static void getFileListing(FileSystem *fs, unsigned char *buf, int maxValueLenAllowed, const char *directoryPath)
{
    int len;
    struct dirent *pDirent;
    DIR *pDir;

    pDir = opendir(directoryPath);
    if(pDir == NULL)
    {
        error_log("Cannot open directory '%s'\n", directoryPath);
        return;
    }

    char firstEntryDone = 0;

    strcat(buf, "{");
    while ((pDirent = readdir(pDir)) != NULL)
    {
        struct stat path_stat;
        stat(pDirent->d_name, &path_stat);

        if(S_ISREG(path_stat.st_mode))
        {
            char newEntry[MAX_BUFFER_SIZE] = {0};
            sprintf(newEntry, "\"%s\":%ld", pDirent->d_name, (long)path_stat.st_size);

            if((strlen(buf) + strlen(newEntry)) < (maxValueLenAllowed - 10))
            {
                if(firstEntryDone == 1)
                {
                    strcat(buf, ",");
                }
                strcat(buf, newEntry);
                firstEntryDone = 1;
            }
            else
            {
                break;
            }
        }
    }
    strcat(buf, "}");
}


static long getFileSize(FileSystem *fs, const char *filepath)
{
    struct stat path_stat;
    stat(filepath, &path_stat);

    return (long)path_stat.st_size;
}


void init_file_system(FileSystem *fs, void *arg)
{
    // Register read-callback.
	fs->read = linux_fs_read;

    // Register write-callback.
	fs->write = linux_fs_write;

    // Register other-callbacks.
    fs->renameFile = renameFile;
    fs->deleteFile = deleteFile;
    fs->getFileListing = getFileListing;
    fs->getFileSize = getFileSize;

    // Connect the medium (file-pointer).
    const char *fileName = (const char*)arg;
    connect_underlying_medium_guaranteed(fs, fileName);
}


void release_file_system(FileSystem *fs)
{
    release_underlying_medium_guaranteed(fs);
}
