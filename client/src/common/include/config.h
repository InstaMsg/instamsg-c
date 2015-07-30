#ifndef INSTAMSG_CONFIG
#define INSTAMSG_CONFIG

#include "../../communication/include/fs.h"
#include "./log.h"

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

void readConfig(Config *config, Logger *logger, const unsigned char *key, ValueType valueType, void *value);

#endif
