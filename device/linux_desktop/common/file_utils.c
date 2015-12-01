#include <stdio.h>
#include <string.h>

#include "../../../common/instamsg/driver/include/globals.h"

void readLine(FILE *fp, char *buffer, int maxBufferLength)
{
    int i = 0;
    char ch;

    memset(buffer, 0, maxBufferLength);

    while(1)
    {
        ch = fgetc(fp);
        if((ch != EOF) && (ch != '\n'))
        {
            buffer[i++] = ch;
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


int appendLine(const char *filePath, char *buffer)
{
    int rc = FAILURE;

    FILE *fp = NULL;
    int i;

    fp = fopen(filePath, "a+");
    if(fp != NULL)
    {
        for(i = 0; i < strlen(buffer); i++)
        {
            fputc(buffer[i], fp);
        }

        fputc('\n', fp);
        fclose(fp);

        rc = SUCCESS;
    }
    else
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "Could not open file %s in append-mode", filePath);
        error_log(LOG_GLOBAL_BUFFER);
    }

    return rc;
}
