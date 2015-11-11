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

void process_config(char *configJson);


void add_editable_config(void *var,
                         const char *key,
                         enum CONFIG_TYPE type,
                         char *stringified_value,
                         char *desc);


#endif
