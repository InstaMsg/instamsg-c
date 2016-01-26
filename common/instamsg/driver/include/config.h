#ifndef INSTAMSG_CONFIG
#define INSTAMSG_CONFIG


enum CONFIG_TYPE
{
    CONFIG_STRING,
    CONFIG_INT
};

#define CONFIG                      "[CONFIG] "
#define CONFIG_ERROR                "[CONFIG_ERROR] "

#define CONFIG_KEY_KEY              "key"
#define CONFIG_TYPE_KEY             "type"
#define CONFIG_VALUE_KEY            "val"
#define CONFIG_DESCRIPTION_KEY      "desc"


void init_config();
int get_config_value_from_persistent_storage(const char *key, char *buffer, int maxBufferLength);
int save_config_value_on_persistent_storage(const char *key, const char *value);
int delete_config_value_from_persistent_storage(const char *key);

void generate_config_json(char *messageBuffer, const char *key, enum CONFIG_TYPE type, const char *stringified_value, const char *desc);
void process_config(char *configJson);



/*
 *********************************************************************************************************************
 **************************************** PUBLIC APIs *****************************************************************
 **********************************************************************************************************************
 */

/*
 * This method registers a configuration, which is then editable at the InstaMsg-Server.
 * Once done, this configuration will be visible/editable in the "Configuration" tab on the Clients-page.
 *
 * The steps for editing and pushing the changed-configuration from server to device, are detailed in the
 * "Configuration" tab.
 *
 *
 * Register-Method-Explanation ::
 * ===============================
 *
 *
 * var                  :
 *
 *      Pointer to the variable, that will store the value.
 *
 *      Depending on whether the variable is of type INTEGER or STRING, the variable (to which the pointer is pointing)
 *      must be of proper type.
 *
 *
 * key                  :
 *
 *      String value, that will serve as the index to this config.
 *
 *
 * type                 :
 *
 *      One of CONFIG_STRING or CONFIG_INT.
 *
 *
 * stringified_value    :
 *
 *      For type of CONFIG_STRING, this will be a simple string.
 *      For type of CONFIG_INT, this will be a stringified integer value.
 *
 *
 * desc                 :
 *
 *      A bried description of what this configuration is for.
 *
 *
 *
 * Please see "static void handleConnOrProvAckGeneric(InstaMsg *c, int connack_rc)" method in
 *          common/instamsg/driver/instamsg.c
 *
 * for simple example on how to register an editable-configuration.
 *
 */
void registerEditableConfig(void *var,
                            const char *key,
                            enum CONFIG_TYPE type,
                            char *stringified_value,
                            char *desc);


#endif
