/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "../../../common/instamsg/driver/include/config.h"
#include "../../../common/instamsg/driver/include/globals.h"
#include "../../../common/instamsg/driver/include/log.h"
#include "../../../common/instamsg/driver/include/json.h"
#include "../../../common/instamsg/driver/include/sg_mem.h"
#include "../../../common/instamsg/driver/include/file_system.h"

#include "../common/file_utils.h"


static char tempBuffer[1024];

#define CONFIG_FILE_NAME "config.txt"


static int get_config_value_from_persistent_storage_and_delete_if_asked(const char *key, char *buffer, int maxBufferLength,
                                                                        unsigned char deleteConfig)
{
    int rc = FAILURE;

    FILE_STRUCT *fp = FILE_OPEN(CONFIG_FILE_NAME, "r");
    char *jsonKey = NULL;

    if(fp == NULL)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "%sCould not open config-file [%s] for reading", CONFIG_ERROR, CONFIG_FILE_NAME);
        error_log(LOG_GLOBAL_BUFFER);

        goto exit;
    }


    jsonKey = (char*) sg_malloc(MAX_BUFFER_SIZE);
    if(jsonKey == NULL)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "%sCould not allocate memory in config.. not proceeding", CONFIG_ERROR);
        error_log(LOG_GLOBAL_BUFFER);

        goto exit;
    }
    memset(jsonKey, 0, MAX_BUFFER_SIZE);

    while(1)
    {
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
           memset(jsonKey, 0, MAX_BUFFER_SIZE);
           getJsonKeyValueIfPresent(tempBuffer, CONFIG_KEY_KEY, jsonKey);

           if(strcmp(jsonKey, key) == 0)
           {
               if(deleteConfig == 0)
               {
                   int i;
                   for(i = 0; i < strlen(tempBuffer); i++)
                   {
                       buffer[i] = tempBuffer[i];
                   }

                   rc = SUCCESS;
               }
               else
               {
               }

               /*
                * We keep looking further, just in case there are multiple-values of the config.
                * If BYYY CHHHANCCE anyone stores multiple values, following will happen ::
                *
                * a)
                * For getting config, the LAST config (for the particular key) will be picked.
                *
                * b)
                * For deleting config, ALL configs (for the particular key) will be deleted.
                */
           }
           else
           {
               if(deleteConfig == 1)
               {
                   /*
                    * Write this non-matching-config to a new-temp file.
                    */
                   sg_appendLine(SYSTEM_WIDE_TEMP_FILE, tempBuffer);
               }
           }
       }
    }

exit:
    if(fp != NULL)
    {
        FILE_CLOSE(fp);
    }

    if(jsonKey != NULL)
    {
        sg_free(jsonKey);
    }

    /*
     * Move the file, if we were in delete-mode.
     */
    if(deleteConfig == 1)
    {
        if(renameFile(NULL, SYSTEM_WIDE_TEMP_FILE, CONFIG_FILE_NAME) != 0)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, "%sCould not move file from [%s] to [%s]", CONFIG_ERROR, SYSTEM_WIDE_TEMP_FILE, CONFIG_FILE_NAME);
            error_log(LOG_GLOBAL_BUFFER);
        }
    }

    return rc;
}


static int do_delete_config_value_from_persistent_storage(const char *key, unsigned char acquireMutex)
{
	int rc = FAILURE;
	if(acquireMutex == 1)
	{
		ACQUIRE_THREAD_MUTEX
	}

    rc = get_config_value_from_persistent_storage_and_delete_if_asked(key, NULL, 0, 1);

    if(acquireMutex == 1)
    {
    	RELEASE_THREAD_MUTEX
    }

    return rc;
}


/*
 * This method initializes the Config-Interface for the device.
 */
void init_config()
{
}


/*
 * This method fills in the JSONified-config-value for "key" into "buffer".
 *
 * It returns the following ::
 *
 * SUCCESS ==> If a config with the specified "key" is found.
 * FAILURE ==> If no config with the specified "key" is found.
 */
int get_config_value_from_persistent_storage(const char *key, char *buffer, int maxBufferLength)
{
	int rc = FAILURE;
	ACQUIRE_THREAD_MUTEX

    rc =  get_config_value_from_persistent_storage_and_delete_if_asked(key, buffer, maxBufferLength, 0);

    RELEASE_THREAD_MUTEX
    return rc;
}


/*
 * This method saves the JSONified-config-value for "key" onto persistent-storage.
 * The example value is of the form ::
 *
 *      {'key' : 'key_value', 'type' : '1', 'val' : 'value', 'desc' : 'description for this config'}
 *
 *
 * Note that for the 'type' field :
 *
 *      '0' denotes that the key-type is of STRING
 *      '1' denotes that the key-type is of INTEGER (although it is stored in stringified-form in 'val' field).
 *
 * It returns the following ::
 *
 * SUCCESS ==> If the config was successfully saved.
 * FAILURE ==> If the config could not be saved.
 */
int save_config_value_on_persistent_storage(const char *key, const char *value, unsigned char logging)
{
	int rc = FAILURE;
	ACQUIRE_THREAD_MUTEX

	do_delete_config_value_from_persistent_storage(key, 0);
    rc = sg_appendLine(CONFIG_FILE_NAME, value);

    RELEASE_THREAD_MUTEX
	return rc;
}


/*
 * This method deletes the JSONified-config-value for "key" (if at all it exists).
 *
 * It returns the following ::
 *
 * SUCCESS ==> If a config with the specified "key" was found and deleted successfully.
 * FAILURE ==> In every other case.
 */
int delete_config_value_from_persistent_storage(const char *key)
{
	return do_delete_config_value_from_persistent_storage(key, 1);
}


/*
 * This method releases the config, just before the system is going for a reset.
 */
void release_config()
{
}
