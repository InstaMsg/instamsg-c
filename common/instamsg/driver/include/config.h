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



#ifndef INSTAMSG_CONFIG
#define INSTAMSG_CONFIG


enum CONFIG_TYPE
{
    CONFIG_STRING,
    CONFIG_INT
};

#define CONFIG                      PROSTR("[CONFIG] ")
#define CONFIG_ERROR                PROSTR("[CONFIG_ERROR] ")

#define CONFIG_KEY_KEY              "key"
#define CONFIG_TYPE_KEY             "type"
#define CONFIG_VALUE_KEY            "val"
#define CONFIG_DESCRIPTION_KEY      "desc"

#define AUTO_UPGRADE_ENABLED        PROSTR("AUTO_UPGRADE_ENABLED")


void init_config();
int get_config_value_from_persistent_storage(const char *key, char *buffer, int maxBufferLength);
int save_config_value_on_persistent_storage(const char *key, const char *value, unsigned char logging);
int delete_config_value_from_persistent_storage(const char *key);

void generate_config_json(char *messageBuffer, const char *key, enum CONFIG_TYPE type, const char *stringified_value, const char *desc);
void process_config(char *configJson, unsigned char persistConfig);
void release_config();



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
