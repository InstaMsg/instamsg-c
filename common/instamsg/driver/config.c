#include <string.h>

#include "include/globals.h"
#include "include/log.h"
#include "include/sg_mem.h"
#include "include/config.h"
#include "include/instamsg.h"
#include "include/sg_stdlib.h"
#include "include/json.h"

static int publishConfig(const char *topicName, const char *message)
{
    return MQTTPublish(topicName,
                       message,
                       QOS1,
                       0,
                       NULL,
                       MQTT_RESULT_HANDLER_TIMEOUT,
                       0,
                       1);
}


void process_config(char *configJson)
{
    char *config_key = NULL;

    config_key = (char*) sg_malloc(sizeof(GLOBAL_BUFFER));
    if(config_key == NULL)
    {
        error_log(CONFIG_ERROR "Could not allocate memory for config-key.");
        goto exit;
    }
    getJsonKeyValueIfPresent(configJson, CONFIG_KEY_KEY, config_key);

    /*
     * Save the config on persistent-storage.
     */
    save_config_value_on_persistent_storage(config_key, configJson);

    /*
     * Finally, publish the config on the server, so that the device and server remain in sync.
     */
    startAndCountdownTimer(1, 0);
    publishConfig(TOPIC_CONFIG_SEND, configJson);

exit:
    if(config_key)
        sg_free(config_key);
}


void add_editable_config(void *var,
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
            error_log(CONFIG_ERROR "Could not allocate memory for stored_value");
            goto exit;
        }
        getJsonKeyValueIfPresent((char*)GLOBAL_BUFFER, CONFIG_VALUE_KEY, stored_value);
        stringified_value = stored_value;

        stored_desc = (char*) sg_malloc(MAX_BUFFER_SIZE);
        if(stored_desc == NULL)
        {
            error_log(CONFIG_ERROR "Could not allocate memory for stored_desc");
            goto exit;
        }
        getJsonKeyValueIfPresent((char*)GLOBAL_BUFFER, CONFIG_DESCRIPTION_KEY, stored_desc);
        desc = stored_desc;

        info_log(CONFIG "Default-config-values overridden by stored-values. Key = [%s], Value = [%s], Description = [%s]",
                 key, stringified_value, desc);
    }


    /*
     * Now, consume the config-value.
     */
    if(type == CONFIG_STRING)
    {
        strcpy((char*)var, stringified_value);
        info_log(CONFIG "Using value [%s] for key [%s] of type STRING", stringified_value, key);
    }
    else
    {
        *((int*)var) = sg_atoi(stringified_value);
        info_log(CONFIG "Using value [%d] for key [%s] of type INTEGER", *((int*)var), key);
    }


    /*
     * Form a Config-JSON, and do the additional-processing required.
     */
    memset(messageBuffer, 0, sizeof(messageBuffer));
    sg_sprintf(messageBuffer, "{'%s' : '%s', '%s' : '%d', '%s' : '%s', '%s' : '%s'}",
               CONFIG_KEY_KEY, key,
               CONFIG_TYPE_KEY, type,
               CONFIG_VALUE_KEY, stringified_value,
               CONFIG_DESCRIPTION_KEY, desc);

    process_config(messageBuffer);

exit:
    if(stored_value)
        sg_free(stored_value);

    if(stored_desc)
        sg_free(stored_desc);
}

