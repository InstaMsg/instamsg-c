#ifndef INSTAMSG_CONFIG
#define INSTAMSG_CONFIG

#include "instamsg_vendor.h"

#include "./log.h"
#include "./globals.h"

typedef struct Config Config;
struct Config
{
    FileSystem fs;
};
Config config;

void init_config(Config *config, void *arg);
void release_config(Config *config);

void readConfig(Config *config, Logger *logger, const unsigned char *key, enum ValueType valueType, void *value);

#endif
