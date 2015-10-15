#include <string.h>

#include "include/globals.h"
#include "include/log.h"
#include "include/sg_mem.h"


/*
 * This is a very restricted version of json-parsing, that involves no malloc/free, and which involves only a python-dict.
 *
 * Upon every parsing-request, the complete json will be parsed.
 * But here (in the embedded-scenario), we want correctness and reliability (speed is kinda secondary).
 *
 * The key/value can be without double-quotes or single-quotes, but we will return them as a string nevertheless
 * (the calling-function will do the necessary conversions as necessary).
 */
void getJsonKeyValueIfPresent(char *json, const char *key, char *buf)
{
    unsigned char NOT_FOUND, keyWrapper;
    char *jsonStartingPointer, *parsedKeyToken, *parsedValueToken, *token;

    parsedKeyToken = (char *)sg_malloc(MAX_BUFFER_SIZE);
    parsedValueToken = (char *)sg_malloc(MAX_BUFFER_SIZE);
    if((parsedKeyToken == NULL) || (parsedValueToken == NULL))
    {
        error_log("Could not allocate memory in getJsonKeyValueIfPresent");
        goto exit;
    }

    memset(parsedKeyToken, 0, MAX_BUFFER_SIZE);
    memset(parsedValueToken, 0, MAX_BUFFER_SIZE);

    NOT_FOUND = ' ';
    keyWrapper = NOT_FOUND;
    jsonStartingPointer = json;
    token = parsedKeyToken;

    while(*json)
    {
        if((keyWrapper == NOT_FOUND) && (*json == NOT_FOUND))
        {
        }
        else if((keyWrapper == NOT_FOUND) && ((*json == '{') || (*json == ',')))
        {
        }
        else if((keyWrapper == NOT_FOUND) && ((*json == '\'') || (*json == '"')))
        {
            /* This means we need to start parsing the key now. */
            keyWrapper = *json;
        }
        else if((*json == ':') && (keyWrapper == NOT_FOUND))
        {
        }
        else if(    (*json == keyWrapper) ||
                    ((*json == '}') && (keyWrapper == NOT_FOUND)))
        {
            /* We need to stop parsing the key now. */
            keyWrapper = NOT_FOUND;

            /* Now, if we were currrently parsing-key, move to parsing value. */
            if((strlen(parsedValueToken) == 0) && (token == parsedKeyToken))
            {
                token = parsedValueToken;
            }
            else
            {
                /* If we found the current key-value, we are done. */
                if(strcmp(parsedKeyToken, key) == 0)
                {
                    strcat(buf, parsedValueToken);

                    debug_log("Found key [%s] and value [%s] in json [%s]", parsedKeyToken, buf, jsonStartingPointer);
                    goto exit;
                }

                /* We have parsed current key-value pair. So, reset the token-buffers. */
                memset(parsedKeyToken, 0, MAX_BUFFER_SIZE);
                memset(parsedValueToken, 0, MAX_BUFFER_SIZE);

                /* Set the current-token to "key"-parsing-mode. */
                token = parsedKeyToken;
            }
        }
        else
        {
            /* Simply add to the running token. */
            char ch[2] = {0};
            ch[0] = *json;
            strcat(token, ch);
        }

        json++;
    }

exit:

    if(parsedKeyToken)
        sg_free(parsedKeyToken);

    if(parsedValueToken)
        sg_free(parsedValueToken);
}

