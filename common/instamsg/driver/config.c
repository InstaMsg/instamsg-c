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

#include "include/globals.h"
#include "include/log.h"
#include "include/sg_mem.h"
#include "include/config.h"
#include "include/instamsg.h"
#include "include/sg_stdlib.h"
#include "include/json.h"

#if FILE_SYSTEM_ENABLED == 1
#include "include/file_system.h"
int write_singular_line_into_file(const char *filePath, const char *buffer);
#endif


static int publishConfig(const char *topicName, const char *message)
{
    return publish(topicName,
                   message,
                   QOS0,
                   0,
                   NULL,
                   MQTT_RESULT_HANDLER_TIMEOUT,
                   1);
}


void generate_config_json(char *messageBuffer, const char *key, enum CONFIG_TYPE type, const char *stringified_value, const char *desc)
{
    sg_sprintf(messageBuffer, PROSTR("{'%s' : '%s', '%s' : '%d', '%s' : '%s', '%s' : '%s'}"),
               CONFIG_KEY_KEY, key,
               CONFIG_TYPE_KEY, type,
               CONFIG_VALUE_KEY, stringified_value,
               CONFIG_DESCRIPTION_KEY, desc);
}


void process_config(char *configJson)
{
    char *config_key = NULL;

    config_key = (char*) sg_malloc(sizeof(GLOBAL_BUFFER));
    if(config_key == NULL)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sCould not allocate memory for config-key."), CONFIG_ERROR);
        error_log(LOG_GLOBAL_BUFFER);

        goto exit;
    }
    memset(config_key, 0, sizeof(GLOBAL_BUFFER));
    getJsonKeyValueIfPresent(configJson, CONFIG_KEY_KEY, config_key);

    /*
     * Save the config on persistent-storage.
     */
    save_config_value_on_persistent_storage(config_key, configJson, 1);

#if FILE_SYSTEM_ENABLED == 1
    {
        if(strcmp(config_key, AUTO_UPGRADE_ENABLED) == 0)
        {
            char *config_value = (char*) sg_malloc(3);
            if(config_value == NULL)
            {
                sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sCould not allocate memory for reading value of config"), CONFIG_ERROR);
                error_log(LOG_GLOBAL_BUFFER);
            }
            else
            {
                memset(config_value, 0, 3);

                getJsonKeyValueIfPresent(configJson, CONFIG_VALUE_KEY, config_value);

                write_singular_line_into_file(SYSTEM_WIDE_TEMP_FILE, config_value);
                renameFile(NULL, SYSTEM_WIDE_TEMP_FILE, "auto_upgrade");

                sg_free(config_value);
            }
        }
    }
#endif

    /*
     * Finally, publish the config on the server, so that the device and server remain in sync.
     */
    startAndCountdownTimer(1, 0);
    publishConfig(TOPIC_CONFIG_SEND, configJson);

exit:
    if(config_key)
        sg_free(config_key);
}


void registerEditableConfig(void *var,
                            const char *key,
                            enum CONFIG_TYPE type,
                            char *stringified_value,
                            char *desc)
{
    int rc = FAILURE;

    char *stored_value = NULL;
    char *stored_desc = NULL;


    /*
     * Check if a config of this key exists in persistent-storage.
     */
    RESET_GLOBAL_BUFFER;
    rc = get_config_value_from_persistent_storage(key, (char*)GLOBAL_BUFFER, sizeof(GLOBAL_BUFFER));

    if(rc == SUCCESS)
    {
        /*
         * Config found on persistent-storage.
         */
        stored_value = (char*) sg_malloc(MAX_BUFFER_SIZE);
        if(stored_value == NULL)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sCould not allocate memory for stored_value"), CONFIG_ERROR);
            error_log(LOG_GLOBAL_BUFFER);

            goto exit;
        }
        memset(stored_value, 0, MAX_BUFFER_SIZE);
        getJsonKeyValueIfPresent((char*)GLOBAL_BUFFER, CONFIG_VALUE_KEY, stored_value);
        stringified_value = stored_value;

        stored_desc = (char*) sg_malloc(MAX_BUFFER_SIZE);
        if(stored_desc == NULL)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sCould not allocate memory for stored_desc"), CONFIG_ERROR);
            error_log(LOG_GLOBAL_BUFFER);

            goto exit;
        }
        memset(stored_desc, 0, MAX_BUFFER_SIZE);
        getJsonKeyValueIfPresent((char*)GLOBAL_BUFFER, CONFIG_DESCRIPTION_KEY, stored_desc);
        desc = stored_desc;

        sg_sprintf(LOG_GLOBAL_BUFFER,
                   PROSTR("%sDefault-config-values overridden by stored-values. Key = [%s], Value = [%s], Description = [%s]"),
                   CONFIG, key, stringified_value, desc);
        info_log(LOG_GLOBAL_BUFFER);
    }


    /*
     * Now, consume the config-value.
     */
    if(type == CONFIG_STRING)
    {
        char *destination_string = (char*) var;

        strcpy(destination_string, stringified_value);
        destination_string[strlen(stringified_value)] = 0;

        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sUsing value [%s] for key [%s] of type STRING"), CONFIG, destination_string, key);
        info_log(LOG_GLOBAL_BUFFER);
    }
    else
    {
        *((int*)var) = sg_atoi(stringified_value);

        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sUsing value [%d] for key [%s] of type INTEGER"), CONFIG, *((int*)var), key);
        info_log(LOG_GLOBAL_BUFFER);
    }


    /*
     * Form a Config-JSON, and do the additional-processing required.
     */
    memset(messageBuffer, 0, sizeof(messageBuffer));
    generate_config_json(messageBuffer, key, type, stringified_value, desc);

    process_config(messageBuffer);

exit:
    if(stored_desc)
        sg_free(stored_desc);

    if(stored_value)
        sg_free(stored_value);
}

