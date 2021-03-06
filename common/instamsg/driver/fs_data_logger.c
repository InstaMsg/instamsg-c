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

#include "device_defines.h"

#if FILE_SYSTEM_ENABLED == 1

#include <stdio.h>
#include <string.h>

#include "./include/data_logger.h"
#include "./include/globals.h"
#include "./include/log.h"
#include "./include/file_system.h"

#define DATA_FILE_NAME      "data.txt"

static char tempBuffer[1024];

static void make_the_switch()
{
    if(1)
    {
        if(sg_renameFile(NULL, SYSTEM_WIDE_TEMP_FILE, DATA_FILE_NAME) != 0)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, "%sCould not move file from [%s] to [%s]", DATA_LOGGING_ERROR, SYSTEM_WIDE_TEMP_FILE, DATA_FILE_NAME);
            error_log(LOG_GLOBAL_BUFFER);
        }
    }
}


/*
 * This method initializes the data-logger-interface for the device.
 */
void fs_init_data_logger()
{
    /*
     * Create the logger-file if not already.
     */
    FILE_STRUCT *fp = FILE_OPEN(DATA_FILE_NAME, "r");
    if(fp == NULL)
    {
        sg_createEmptyFile(DATA_FILE_NAME);
    }
    else
    {
        FILE_CLOSE(fp);
    }
}


/*
 * This method saves the record on the device.
 *
 * If and when the device-storage becomes full, the device MUST delete the oldest record, and instead replace
 * it with the current record. That way, we will maintain a rolling-data-logger.
 */
void fs_save_record_to_persistent_storage(char *record)
{
    while(1)
    {
        long currentFileSize = getFileSize(NULL, DATA_FILE_NAME);

        if(currentFileSize < MAX_DATA_LOGGER_SIZE_BYTES)
        {
            break;
        }
        else
        {
            /*
             * Remove the first (oldest) record.
             */
            FILE_STRUCT *fp = NULL;
            unsigned char firstLineIgnored = 0;

            sg_sprintf(LOG_GLOBAL_BUFFER, "%sRemoving the oldest record ....", DATA_LOGGING);
            error_log(LOG_GLOBAL_BUFFER);

            fp = FILE_OPEN(DATA_FILE_NAME, "r");
            if(fp != NULL)
            {
                while(1)
                {
                    memset(tempBuffer, 0, sizeof(tempBuffer));
                    sg_readLine(fp, tempBuffer, sizeof(tempBuffer));

                    if(strlen(tempBuffer) == 0)
                    {
                        break;
                    }
                    else if(firstLineIgnored == 1)
                    {
                        sg_appendLine(SYSTEM_WIDE_TEMP_FILE, tempBuffer);
                    }

                    firstLineIgnored = 1;
                }

                FILE_CLOSE(fp);

                if(sg_renameFile(NULL, SYSTEM_WIDE_TEMP_FILE, DATA_FILE_NAME) != 0)
                {
                    sg_sprintf(LOG_GLOBAL_BUFFER, "%sCould not move file from [%s] to [%s]",
                               DATA_LOGGING_ERROR, SYSTEM_WIDE_TEMP_FILE, DATA_FILE_NAME);
                    error_log(LOG_GLOBAL_BUFFER);
                }
            }
            else
            {
                sg_sprintf(LOG_GLOBAL_BUFFER, "%sCould not open file [%s] for reading", DATA_LOGGING_ERROR, DATA_FILE_NAME);
                error_log(LOG_GLOBAL_BUFFER);
            }
        }
    }

    sg_copyFile(NULL, DATA_FILE_NAME, SYSTEM_WIDE_TEMP_FILE);
    sg_appendLine(SYSTEM_WIDE_TEMP_FILE, record);
    make_the_switch();
}


/*
 * The method returns the next available record.
 * If a record is available, following must be done ::
 *
 * 1)
 * The record must be deleted from the storage-medium (so as not to duplicate-fetch this record later).
 *
 * 2)
 * Then actually return the record.
 *
 * Obviously, there is a rare chance that step 1) is finished, but step 2) could not run to completion.
 * That would result in a data-loss, but we are ok with it, because we don't want to send duplicate-records to InstaMsg-Server.
 *
 * We could have done step 2) first and then step 1), but in that scenario, we could have landed in a scenario where step 2)
 * was done but step 1) could not be completed. That could have caused duplicate-data on InstaMsg-Server, but we don't want
 * that.
 *
 *
 * One of the following statuses must be returned ::
 *
 * a)
 * SUCCESS, if a record is successfully returned.
 *
 * b)
 * FAILURE, if no record is available.
 */
int fs_get_next_record_from_persistent_storage(char *buffer, int maxLength)
{
    int rc = FAILURE;
    unsigned char lineRead = 0;

    FILE_STRUCT *fp = FILE_OPEN(DATA_FILE_NAME, "r");
    if(fp == NULL)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "%sCould not open data-log-file [%s] for reading", DATA_LOGGING_ERROR, DATA_FILE_NAME);
        error_log(LOG_GLOBAL_BUFFER);

        goto exit;
    }

    sg_createEmptyFile(SYSTEM_WIDE_TEMP_FILE);

    while(1)
    {

        memset(tempBuffer, 0, sizeof(tempBuffer));
        sg_readLine(fp, tempBuffer, sizeof(tempBuffer));

        if(strlen(tempBuffer) == 0)
        {
            /*
             * Nothing else was left to read.
             */
            goto exit;
        }
        else
        {
            if(lineRead == 0)
            {
                /*
                 * This is the data we want.
                 */
                memcpy(buffer, tempBuffer, strlen(tempBuffer));
                lineRead = 1;

                rc = SUCCESS;
            }
            else
            {
                /*
                 * Copy to temp-file.
                 */
                sg_appendLine(SYSTEM_WIDE_TEMP_FILE, tempBuffer);
            }
        }
    }

	{
		unsigned int i = 0;
		unsigned char onlyWhiteSpace = 1;

		for (i = strlen(buffer) - 1; i >= 0; i--)
		{
			if ((buffer[i] != ' ') && (buffer[i] != '\r') && (buffer[i] != '\n'))
			{
				onlyWhiteSpace = 0;
				break;
			}
		}

		if (onlyWhiteSpace == 1)
		{
			buffer[0] = 0;
		}
	}
exit:
    if(fp != NULL)
    {
        FILE_CLOSE(fp);
    }

    if(1)
    {
        if(sg_renameFile(NULL, SYSTEM_WIDE_TEMP_FILE, DATA_FILE_NAME) != 0)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, "%sCould not move file from [%s] to [%s]", DATA_LOGGING_ERROR, SYSTEM_WIDE_TEMP_FILE, DATA_FILE_NAME);
            error_log(LOG_GLOBAL_BUFFER);
        }
    }

    return rc;
}


/*
 * This method releases the data-logger, just before the system is going for a reset.
 */
void fs_release_data_logger()
{
}

#endif
