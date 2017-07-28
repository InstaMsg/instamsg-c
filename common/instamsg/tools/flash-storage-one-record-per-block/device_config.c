/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/




#include "../../../common/instamsg/driver/include/globals.h"
#include "../../../common/instamsg/driver/include/log.h"
#include "../../../common/instamsg/driver/include/config.h"
#include "../../../common/instamsg/driver/include/sg_mem.h"
#include "../../../common/instamsg/driver/include/json.h"

#include "../common/storage_utils.h"

#include <string.h>

#define CONFIG_TYPE                         "CONFIG"


/**********************************************************************************
 *
 * TODO: ADD/REPLACE THIS FILE AT device/name/instamsg/device_config.c
 *
 *********************************************************************************/

static char tempBuffer[MAX_BUFFER_SIZE];

#define INVALID_RECORD_NUMBER                 -1


static int get_config_value_from_persistent_storage_and_delete_if_asked(const char *key, char *buffer, int maxBufferLength,
                                                                        unsigned char deleteConfig, int ignoreRecordNumber)
{
    int rc = FAILURE;
    int i;
    char *jsonKey = NULL;

    jsonKey = (char*) sg_malloc(MAX_BUFFER_SIZE);
    if(jsonKey == NULL)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "%sCould not allocate memory in config.. not proceeding", CONFIG_ERROR);
        error_log(LOG_GLOBAL_BUFFER);

        goto exit;
    }

    for(i = CONFIG_RECORDS_LOWER_INDEX; i <= CONFIG_RECORDS_UPPER_INDEX; i++)
    {
        if(is_record_valid(i) == SUCCESS)
        {
            memset(tempBuffer, 0, sizeof(tempBuffer));
            read_record_from_persistent_storage(i, tempBuffer, sizeof(tempBuffer), CONFIG_TYPE);

            if(strlen(tempBuffer) == 0)
            {
                continue;
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
                    }
                    else
                    {
                        if(i != ignoreRecordNumber)
                        {
                            mark_record_invalid(i);
                        }
                    }

                    rc = SUCCESS;

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
                    continue;
                }
            }
        }
    }

exit:
    if(jsonKey != NULL)
    {
        sg_free(jsonKey);
    }

    return rc;
}


/*
 * This method initializes the Config-Interface for the device.
 */
void init_config()
{
    init_persistent_storage();
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
    return get_config_value_from_persistent_storage_and_delete_if_asked(key, buffer, maxBufferLength, 0, INVALID_RECORD_NUMBER);
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
    int i = 0, rc = FAILURE;

    for(i = CONFIG_RECORDS_LOWER_INDEX; i <= CONFIG_RECORDS_UPPER_INDEX; i++)
    {
        /*
         * Search for the first available record.
         */
        if(is_record_valid(i) == FAILURE)
        {
            if(logging == 1)
            {
                sg_sprintf(LOG_GLOBAL_BUFFER, "%sStoring config-json [%s] at record-index [%u]", CONFIG, value, i);
                info_log(LOG_GLOBAL_BUFFER);
            }

            rc = write_record_on_persistent_storage(i, value, CONFIG_TYPE);
            if(rc == SUCCESS)
            {
                rc = get_config_value_from_persistent_storage_and_delete_if_asked(key, NULL, 0, 1, i);
                if(rc == FAILURE)
                {
                    sg_sprintf(LOG_GLOBAL_BUFFER, "%sError while deleting older-config-values for key [%s]", CONFIG_ERROR, key);
                    error_log(LOG_GLOBAL_BUFFER);
                }

                return rc;
            }
            else
            {
                sg_sprintf(LOG_GLOBAL_BUFFER, "%sError while writing config key-value [%s]-[%s]", CONFIG_ERROR, key, value);
                error_log(LOG_GLOBAL_BUFFER);

                return rc;
            }
        }
    }

    sg_sprintf(LOG_GLOBAL_BUFFER, "%sNo more space left to hold any config-values. Maximum config-values allowed = [%u]",
               CONFIG_ERROR, CONFIG_RECORDS_UPPER_INDEX - CONFIG_RECORDS_LOWER_INDEX + 1);
    error_log(LOG_GLOBAL_BUFFER);

    return FAILURE;
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
    return get_config_value_from_persistent_storage_and_delete_if_asked(key, NULL, 0, 1, INVALID_RECORD_NUMBER);
}


/*
 * This method releases the config, just before the system is going for a reset.
 */
void release_config()
{
}
