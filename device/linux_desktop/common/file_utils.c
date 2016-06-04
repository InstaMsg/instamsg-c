#include <stdio.h>
#include <string.h>

#include "../../../common/instamsg/driver/include/globals.h"

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
        int lastIndex = strlen(buffer - 1);
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


int sg_appendLine(const char *filePath, char *buffer)
{
    int rc = FAILURE;

    FILE_STRUCT *fp = NULL;
    int i;

    fp = FILE_OPEN(filePath, "a+");
    if(fp != NULL)
    {
        for(i = 0; i < strlen(buffer); i++)
        {
            FILE_PUTC(buffer[i], fp);
        }

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
