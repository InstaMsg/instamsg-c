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

void init_config(Config *config, void *arg)
{
    // Here, physical medium is a file-system.
	init_file_system(&(config->fs), arg);
}


void release_config(Config *config)
{
    release_file_system(&(config->fs));
}


void readConfig(Config *config, const unsigned char *key, enum ValueType valueType, void *value)
{
    rewind((config->fs).fp);

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

            (config->fs).read(&(config->fs), &ch, 1, 1);


            // We reached the end of file, but we were not done yet :(
            if(ch == EOF)
            {
                error_log("[%s] could not be read from config :( ", key);
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
