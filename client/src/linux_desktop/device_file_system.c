#ifdef FILE_SYSTEM_INTERFACE_ENABLED

#include "./device_file_system.h"
#include "../common/include/globals.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <sys/stat.h>


/*
 * This method MUST connect the underlying medium (even if it means to retry continuously).
 */
void connect_underlying_file_system_medium_guaranteed(FileSystem* fs)
{
    fs->fp = fopen(fs->fileName, "a+");
}


/*
 * This method reads first "len" bytes from file-stream into "buffer".
 *
 * This is a blocking function. So, either of the following must hold true ::
 *
 * a)
 * "len" bytes are read.
 * In this case, SUCCESS must be returned.
 *
 *                      OR
 * b)
 * An error occurred while reading.
 * In this case, FAILURE must be returned immediately.
 */
int file_system_read(FileSystem *fs, unsigned char* buffer, int len, unsigned char guaranteed)
{
    if(fread(buffer, sizeof(buffer[0]), len, fs->fp) < len)
    {
        return FAILURE;
    }

    return SUCCESS;
}


/*
 * This method writes first "len" bytes from "buffer" onto the network.
 *
 * This is a blocking function. So, either of the following must hold true ::
 *
 * a)
 * All "len" bytes are written.
 * In this case, SUCCESS must be returned.
 *
 *                      OR
 * b)
 * An error occurred while writing.
 * In this case, FAILURE must be returned immediately.).
 */
int file_system_write(FileSystem *fs, unsigned char* buffer, int len)
{
    if(fwrite(buffer, sizeof(buffer[0]), len, fs->fp) < len)
    {
        return FAILURE;
    }
    fflush(fs->fp);

    return SUCCESS;
}


/*
 * This method renames/moves a file on the file-system.
 */
int renameFile(FileSystem *fs, const char *oldPath, const char *newPath)
{
    return rename(oldPath, newPath);
}


/*
 * This method deletes a file from the file-system.
 */
int deleteFile(FileSystem *fs, const char *filePath)
{
    return remove(filePath);
}


/*
 * This method returns the file-listing in the directory specified, and populates the "buf".
 * The format is ::
 *
 *          {"publisher.sh":152,"subscriber.sh":140,"filetester.sh":137,"config.txt_filetester":148,"stdoutsub":56040,"build_ubuntu_14_04.sh":2892,"README.md":1502,"config.txt":128,"stdoutsub.c":6649,"config.txt_local_testing":128}
 */
void getFileListing(FileSystem *fs, char *buf, int maxValueLenAllowed, const char *directoryPath)
{
    int len;
    struct dirent *pDirent;
    DIR *pDir;
    char firstEntryDone = 0;

    pDir = opendir(directoryPath);
    if(pDir == NULL)
    {
        error_log("Cannot open directory '%s'\n", directoryPath);
        return;
    }


    strcat(buf, "{");
    while ((pDirent = readdir(pDir)) != NULL)
    {
        struct stat path_stat;
        stat(pDirent->d_name, &path_stat);

        if(S_ISREG(path_stat.st_mode))
        {
            char newEntry[MAX_BUFFER_SIZE] = {0};
            sg_sprintf(newEntry, "\"%s\":%u", pDirent->d_name, (unsigned int)path_stat.st_size);

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


/*
 * This method returns a long-value, specifying the size of file in bytes.
 */
long getFileSize(FileSystem *fs, const char *filepath)
{
    struct stat path_stat;
    stat(filepath, &path_stat);

    return (long)path_stat.st_size;
}


/*
 * This method MUST release the underlying medium (even if it means to retry continuously).
 */
void release_underlying_file_system_medium_guaranteed(FileSystem* fs)
{
    FILE *fp = fs->fp;
    if(fp != NULL)
    {
        fclose(fp);
        fp = NULL;      /* This helps to keep track if a file is closed already. */
    }
}

#endif

