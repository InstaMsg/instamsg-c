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


#include <string.h>

#include "device_file_system.h"

#if FILE_SYSTEM_ENABLED == 1

#include "./include/file_system.h"
#include "./include/log.h"
#include "./include/globals.h"

void init_file_system(FileSystem *fs, void *arg)
{
    fs->getFileSize = getFileSize;

    fs->fileName = (char*)arg;
    fs->fp = FILE_OPEN(fs->fileName, "a+");
}


int sg_file_read(FileSystem *fs, unsigned char* buffer, int len, unsigned char guaranteed)
{
    if(FILE_READ_LINE(buffer, sizeof(buffer[0]), len, fs->fp) < len)
    {
        return FAILURE;
    }

    return SUCCESS;
}


int sg_file_write(FileSystem *fs, unsigned char* buffer, int len)
{
    if(FILE_WRITE_LINE(buffer, sizeof(buffer[0]), len, fs->fp) < len)
    {
        return FAILURE;
    }

    FILE_FLUSH(fs->fp);
    return SUCCESS;
}

static char tempBuffer[MAX_BUFFER_SIZE];
int sg_copyFile(FileSystem *fs, const char *oldPath, const char *newPath)
{
	FILE_STRUCT *src = NULL;

    src = FILE_OPEN(oldPath, "r");
    if(src != NULL)
    {
       if(1)
       {
            while(1)
            {
                memset(tempBuffer, 0, sizeof(tempBuffer));
                sg_readLine(src, tempBuffer, sizeof(tempBuffer));

                if(strlen(tempBuffer) == 0)
                {
                    break;
                }
                else
                {
                	sg_appendLine(newPath, tempBuffer);
                }
            }
       }

       FILE_CLOSE(src);
    }

    return SUCCESS;
}


int sg_renameFile(FileSystem *fs, const char *oldPath, const char *newPath)
{
    return FILE_RENAME(oldPath, newPath);
}


int sg_deleteFile(FileSystem *fs, const char *filePath)
{
    return FILE_DELETE(filePath);
}


void release_file_system(FileSystem *fs)
{
    FILE_STRUCT *fp = fs->fp;
    if(fp != NULL)
    {
        FILE_CLOSE(fp);
        fp = NULL;      /* This helps to keep track if a file is closed already. */
    }
}


void sg_readLine(FILE_STRUCT *fp, char *buffer, int maxBufferLength)
{
    int i = 0;
    int ch;

    memset(buffer, 0, maxBufferLength);

    while(1)
    {
        ch = FILE_GETC(fp);
        if((ch != FILE_END_ID) && (ch != '\n'))
        {
            buffer[i++] = (char)ch;
        }
        else
        {
            break;
        }
    }

    /*
     * Remove any trailing \r, if any.
     */
    while(1)
    {
        int lastIndex = strlen(buffer) - 1;
        if(buffer[lastIndex] == '\r')
        {
            buffer[lastIndex] = 0;
        }
        else
        {
            break;
        }
    }
}


void sg_readFile(const char *filePath, char *buffer, int maxBufferLength)
{
    FILE_STRUCT *fp = NULL;
    int i = 0;
    int ch;

    fp = FILE_OPEN(filePath, "r");
    if(fp != NULL)
    {
        memset(buffer, 0, maxBufferLength);
        while(1)
        {
            ch = FILE_GETC(fp);
            if(ch != FILE_END_ID)
            {
                buffer[i++] = (char)ch;
            }
            else
            {
                break;
            }
        }

        FILE_CLOSE(fp);
    }
    else
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "Could not open file [%s] in read-mode", filePath);
        error_log(LOG_GLOBAL_BUFFER);

        return;
    }
}


void sg_writeFile(const char *filePath, char *buffer)
{
    FILE_STRUCT *fp = NULL;

    fp = FILE_OPEN(filePath, "w");
    if(fp != NULL)
    {
    	FILE_WRITE_LINE(buffer, 1, strlen(buffer), fp);
        FILE_CLOSE(fp);
    }
    else
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "Could not open file [%s] in write-mode", filePath);
        error_log(LOG_GLOBAL_BUFFER);

        return;
    }
}


int sg_appendLine(const char *filePath, const char *buffer)
{
    int rc = FAILURE;
    FILE_STRUCT *fp = NULL;

    fp = FILE_OPEN(filePath, "a+");
    if(fp != NULL)
    {
    	FILE_WRITE_LINE(buffer, 1, strlen(buffer), fp);
        FILE_PUTC('\n', fp);
        FILE_CLOSE(fp);

        rc = SUCCESS;
    }
    else
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "Could not open file %s in append-mode", filePath);
        error_log(LOG_GLOBAL_BUFFER);
    }

    return rc;
}


int sg_createEmptyFile(const char *filePath)
{
    int rc = FAILURE;

    FILE_STRUCT *fp = NULL;
    fp = FILE_OPEN(filePath, "w");
    if(fp != NULL)
    {
        FILE_CLOSE(fp);

        rc = SUCCESS;
    }
    else
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "Could not open file %s in write-mode", filePath);
        error_log(LOG_GLOBAL_BUFFER);
    }

    return rc;
}


void read_singular_line_from_file(const char *filePath, const char *content, char *buffer, int maxbufferlength)
{
    FILE_STRUCT *fp = FILE_OPEN(filePath, "r");
    if(fp != NULL)
    {
        sg_readLine(fp, buffer, maxbufferlength);
        FILE_CLOSE(fp);
    }
    else
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("File [%s] does not exist, so %s cannot be read"), filePath, content);
        error_log(LOG_GLOBAL_BUFFER);
    }
}


int write_singular_line_into_file(const char *filePath, const char *buffer)
{
    int rc = FAILURE;
    FILE_STRUCT *fp = NULL;

    fp = FILE_OPEN(filePath, "w");
    if(fp != NULL)
    {
    	FILE_WRITE_LINE(buffer, 1, strlen(buffer), fp);
        FILE_CLOSE(fp);

        rc = SUCCESS;
    }
    else
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "Could not open file %s in write-mode", filePath);
        error_log(LOG_GLOBAL_BUFFER);
    }

    return rc;
}

#endif
