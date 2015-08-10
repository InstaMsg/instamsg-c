#include <stdio.h>
#include <string.h>

#include "include/globals.h"
#include "include/log.h"


/*
 * This is a very restricted version of json-parsing, that involves no malloc/free, and which involves only a python-dict.
 *
 * Upon every parsing-request, the complete json will be parsed.
 * But here (in the embedded-scenario), we want correctness and reliability (speed is kinda secondary).
 *
 * The key/value can be without double-quotes or single-quotes, but we will return them as a string nevertheless
 * (the calling-function will do the necessary conversions as necessary).
 */
void getJsonKeyValueIfPresent(unsigned char *json, const unsigned char *key, unsigned char *buf)
{
    unsigned char NOT_FOUND = ' ';
    unsigned char *jsonStartingPointer = json;

    unsigned char parsedKeyToken[MAX_BUFFER_SIZE] = {0};
    unsigned char parsedValueToken[MAX_BUFFER_SIZE] = {0};

    unsigned char keyWrapper = NOT_FOUND;
    unsigned char *token = parsedKeyToken;

    while(*json)
    {
        if((keyWrapper == NOT_FOUND) && (*json == NOT_FOUND) && (strlen(parsedKeyToken) == 0))
        {
        }
        else if((keyWrapper == NOT_FOUND) && ((*json == '{') || (*json == ',')))
        {
        }
        else if((keyWrapper == NOT_FOUND) && ((*json == '\'') || (*json == '"')))
        {
            // This means we need to start parsing the key now.
            keyWrapper = *json;
        }
        else if(    (*json == keyWrapper) ||
                    ((*json == ':') && (keyWrapper == NOT_FOUND)) ||
                    ((*json == '}') && (keyWrapper == NOT_FOUND)))
        {
            // We need to stop parsing the key now.
            keyWrapper = NOT_FOUND;

            // Now, if we were currrently parsing-key, move to parsing value.
            if(strlen(parsedValueToken) == 0)
            {
                token = parsedValueToken;
            }
            else
            {
                // If we found the current key-value, we are done.
                if(strcmp(parsedKeyToken, key) == 0)
                {
                    strcat(buf, parsedValueToken);

                    debug_log("Found key [%s] and value [%s] in json [%s]", parsedKeyToken, parsedValueToken, jsonStartingPointer);
                    return;
                }

                // We have parsed current key-value pair. So, reset the token-buffers.
                memset(parsedKeyToken, 0, MAX_BUFFER_SIZE);
                memset(parsedValueToken, 0, MAX_BUFFER_SIZE);

                // Set the current-token to "key"-parsing-mode.
                token = parsedKeyToken;
            }
        }
        else
        {
            // Simply add to the running token.
            char ch[2] = {0};
            ch[0] = *json;
            strcat(token, ch);
        }

        json++;
    }
}

/*
int main()
{
    while(1)
    {
        char value[1000] = {0};

        getJsonKeyValueIfPresent("{'ajay' :  gatg, ajat:garg}", "ajat", value);
        printf("%s\n", value);

        memset(value, 0, 1000);
        getJsonKeyValueIfPresent("{'ajay' :  gatg, ajat:garg}", "ajay", value);
        printf("%s\n", value);

        break;
    }

    return 0;
}
*/
