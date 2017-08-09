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

