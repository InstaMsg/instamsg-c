#ifndef INSTAMSG_CONFIG
#define INSTAMSG_CONFIG

#include "../../communication/include/fs.h"

typedef struct Config Config;
struct Config
{
    FileSystem *medium;
};

typedef enum ValueType ValueType;
enum ValueType
{
    STRING = 0,
    INTEGER
};

Config* get_new_config(void *arg);
void release_config(Config*);

void readConfig(Config *config, const unsigned char *key, ValueType valueType, void *value);

#endif
