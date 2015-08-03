#ifndef INSTAMSG_CONFIG
#define INSTAMSG_CONFIG

#include "./log.h"
#include "./globals.h"
#include "../../communication/include/fs.h"

typedef struct Config Config;
struct Config
{
    FileSystem *medium;
};

Config* get_new_config(void *arg);
void release_config(Config*);

void readConfig(Config *config, Logger *logger, const unsigned char *key, enum ValueType valueType, void *value);

Config *config;

#endif
