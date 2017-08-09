/*******************************************************************************
 *
 * Copyright (c) 2014 SenseGrow, Inc.
 *
 * SenseGrow Internet of Things (IoT) Client Frameworks
 * http://www.sensegrow.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.

 * Contributors:
 *    Ajay Garg <ajay.garg@sensegrow.com>
 *******************************************************************************/



#include "./device_file_system.h"

#include "../../../common/instamsg/driver/include/globals.h"
#include "../../../common/instamsg/driver/include/log.h"

#if FILE_SYSTEM_ENABLED == 1

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>


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
        sg_sprintf(LOG_GLOBAL_BUFFER, "Cannot open directory '%s'\n", directoryPath);
        error_log(LOG_GLOBAL_BUFFER);

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
 * Gets the first available file-path in the folder
 */
void getNextFileToProcessPath(char *folder, char *path, int maxBufferLength)
{
    struct dirent *pDirent;
    DIR *pDir;

    pDir = opendir(folder);
    if(pDir == NULL)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "Cannot open directory [%s]", folder);
        error_log(LOG_GLOBAL_BUFFER);

        return;
    }

    while((pDirent = readdir(pDir)) != NULL)
    {
        struct stat path_stat;
        stat(pDirent->d_name, &path_stat);

        if( (strcmp(pDirent->d_name, ".") != 0) && (strcmp(pDirent->d_name, "..") != 0) )
        {
            sg_sprintf(path, "%s%s%s", folder, SEPARATOR, pDirent->d_name);
            break;
        }
        else
        {
            continue;
        }
    }

    if(pDir != NULL)
    {
        closedir(pDir);
    }
}

#endif

