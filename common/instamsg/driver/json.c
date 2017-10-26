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



#include <string.h>

#include "include/globals.h"
#include "include/log.h"
#include "include/sg_mem.h"
#include "include/json.h"

#if SSL_ENABLED == 1
#define JSON_BUFFER_SIZE            SSL_JSON_BUFFER_SIZE
#define JSON_VALUE_BUFFER_SIZE      SSL_JSON_VALUE_BUFFER_SIZE
#else
#define JSON_BUFFER_SIZE            MAX_BUFFER_SIZE
#define JSON_VALUE_BUFFER_SIZE      MAX_BUFFER_SIZE
#endif


/*
 * This is a very restricted version of json-parsing, that involves no malloc/free, and which involves only a python-dict.
 *
 * Upon every parsing-request, the complete json will be parsed.
 * But here (in the embedded-scenario), we want correctness and reliability (speed is kinda secondary).
 *
 * The key/value can be without double-quotes or single-quotes, but we will return them as a string nevertheless
 * (the calling-function will do the necessary conversions as necessary).
 */
void getJsonKeyValueIfPresent(char *json_original, const char *key, char *buf)
{
    unsigned char NOT_FOUND, keyWrapper;
    char *parsedKeyToken = NULL, *parsedValueToken = NULL, *token = NULL;
    char *json = NULL, *newJsonBeginnerPointer = NULL;

    json = (char*) sg_malloc(JSON_BUFFER_SIZE);
    if(json == NULL)
    {
        error_log("Could not allocate mmeory for JSON");
        goto exit;
    }
    memset(json, 0, JSON_BUFFER_SIZE);
    strcpy(json, json_original);
    newJsonBeginnerPointer = json;

    parsedKeyToken = (char *)sg_malloc(MAX_BUFFER_SIZE);
    parsedValueToken = (char *)sg_malloc(JSON_VALUE_BUFFER_SIZE);
    if((parsedKeyToken == NULL) || (parsedValueToken == NULL))
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Could not allocate memory in getJsonKeyValueIfPresent"));
        error_log(LOG_GLOBAL_BUFFER);

        goto exit;
    }
    memset(parsedKeyToken, 0, MAX_BUFFER_SIZE);
    memset(parsedValueToken, 0, JSON_VALUE_BUFFER_SIZE);

    NOT_FOUND = ' ';
    keyWrapper = NOT_FOUND;
    token = parsedKeyToken;

    /*
     * Replace all \r and \n with NOT_FOUND.
     */
    {
		unsigned int i;
        for(i = 0; i < strlen(json); i++)
        {
            if((json[i] == '\r') || (json[i] == '\n'))
            {
                json[i] = NOT_FOUND;
            }
        }
    }

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
        else if((keyWrapper == NOT_FOUND) && (*json == ':'))
        {
        }
        else if((keyWrapper == NOT_FOUND) && (*json != '}'))
        {
            keyWrapper = ',';

            {
                /* Simply add to the running token. */
                char ch[2] = {0};
                ch[0] = *json;
                strcat(token, ch);
            }
        }
        else if(    (keyWrapper == *json) ||
                    ((keyWrapper == NOT_FOUND) && (*json == '}')))
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

    if(parsedValueToken)
        sg_free(parsedValueToken);

    if(parsedKeyToken)
        sg_free(parsedKeyToken);

    if(newJsonBeginnerPointer)
        sg_free(newJsonBeginnerPointer);

}


void get_inner_outer_json_from_two_level_json(const char *nestedJson, const char *nestedJsonKey,
                                              char *outerJsonBuffer, int outerJsonBufferLength,
                                              char *innerJsonBuffer, int innerJsonBufferLength)
{
    char *nestedJsonCopy = NULL;

    nestedJsonCopy = (char*) sg_malloc(strlen(nestedJson) + 1);
    if(nestedJsonCopy == NULL)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("Could not allocate memory"));
        error_log(LOG_GLOBAL_BUFFER);

        goto exit;
    }

    strcpy(nestedJsonCopy, nestedJson);

    if(1)
    {
        char *firstOpeningBrace = strstr(nestedJsonCopy, "{");
        if(firstOpeningBrace != NULL)
        {
            char *secondOpeningBrace = strstr(firstOpeningBrace + 1, "{");
            if(secondOpeningBrace != NULL)
            {
                char *secondClosingBrace = strstr(secondOpeningBrace + 1, "}");
                if(secondClosingBrace != NULL)
                {
                    char *nestedJsonKeyBeginner = NULL;

                    memset(innerJsonBuffer, 0, innerJsonBufferLength);
                    memmove(innerJsonBuffer, secondOpeningBrace, secondClosingBrace - secondOpeningBrace + 1);

                    nestedJsonKeyBeginner = strstr(nestedJsonCopy, nestedJsonKey);
                    if(nestedJsonKeyBeginner != NULL)
                    {
                        int lenFromSecondClosingBrace = 0;

                        char *tmp = strstr(secondClosingBrace + 1, ",");
                        if(tmp != NULL)
                        {
                            secondClosingBrace = tmp;
                        }

                        lenFromSecondClosingBrace = strlen(secondClosingBrace + 1);

                        memmove(nestedJsonKeyBeginner, secondClosingBrace + 1, lenFromSecondClosingBrace);
                        nestedJsonKeyBeginner[lenFromSecondClosingBrace] = 0;

                        memset(outerJsonBuffer, 0, outerJsonBufferLength);
                        strcpy(outerJsonBuffer, nestedJsonCopy);
                    }
                }
            }
        }
    }

exit:
    if(nestedJsonCopy != NULL)
    {
        sg_free(nestedJsonCopy);
    }
}

