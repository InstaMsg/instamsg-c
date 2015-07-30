/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/




#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./include/config.h"
#include "./include/globals.h"

Config* get_new_config(void *arg)
{
    Config *config = (Config*)malloc(sizeof(Config));

    // Here, physical medium is a file-system.
	config->medium = get_new_file_system(arg);

    return config;
}


void release_config(Config *config)
{
    release_file_system(config->medium);

    // Free the dynamically-allocated memory
    if(config != NULL)
    {
        free(config);
    }
}


void readConfig(Config *config, Logger *logger, const unsigned char *key, ValueType valueType, void *value)
{
    rewind(config->medium->medium);

    while(1)
    {
        char candidateKey[MAX_BUFFER_SIZE] = {0};
        char candidateValue[MAX_BUFFER_SIZE] = {0};
        int keyLen = 0;
        int valueLen = 0;

        unsigned char equalitySignReached = 0;

        while(1)
        {
            char ch = EOF;

            config->medium->read(config->medium, &ch, 1);


            // We reached the end of file, but we were not done yet :(
            if(ch == EOF)
            {
                error_log(logger, "[%s] could not be read from config :( ", key);
                return;
            }

            // We have reached till the end of line.
            if(ch == '\n')
            {
                break;
            }

            // Keep filling the key- and value-buffer on the run
            if(ch == '=')
            {
                equalitySignReached = 1;
                continue;
            }

            if(equalitySignReached == 0)
            {
                candidateKey[keyLen] = ch;
                keyLen++;
            }
            else
            {
                candidateValue[valueLen] = ch;
                valueLen++;
            }
        }

        if(strcmp(candidateKey, key) == 0)
        {
            if(valueType == INTEGER)
            {
                *((int*)value) = atoi(candidateValue);
            }
            else if(valueType == STRING)
            {
                strcpy((char*)value, candidateValue);
            }

            return;
        }

        // Else, keep trying ..
    }

}

/*
int main()
{
    Logger *logger = get_new_logger("./test_logger");
    Config *config = get_new_config("./config.txt");

    char value[MAX_BUFFER_SIZE] = {0};
    readConfig(config, logger, "ajay", STRING, value);
    printf("Value read == [%s]\n", value);

    int age;
    readConfig(config, logger, "age", INTEGER, &age);
    printf("Value read == [%d]\n", age);
}
*/
