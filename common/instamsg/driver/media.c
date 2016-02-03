#include "./include/media.h"
#include "./include/log.h"
#include "./include/config.h"

#include <string.h>

static char temp[MAX_BUFFER_SIZE];

#define HOST PROSTR("~HOST~")
#define PORT PROSTR("~PORT~")

void processGstTypeUrl(char *host, char *port)
{
    memset(mediaUrl, 0, sizeof(mediaUrl));

    registerEditableConfig(mediaUrl,
                           PROSTR("MEDIA_STREAMING_URL"),
                           CONFIG_STRING,
                           "",
                           PROSTR("Media-URL"));

    if(strlen(mediaUrl) > 0)
    {
        /*
         * Substitute the ~HOST~ and ~PORT~ tokens (if at all present).
         */
        char *ptrHost, *ptrPort;

        memset(temp, 0, sizeof(MAX_BUFFER_SIZE));

        ptrHost = strstr(mediaUrl, HOST);
        ptrPort = strstr(mediaUrl, PORT);

        if((ptrHost == NULL) && (ptrPort == NULL))
        {
            strcpy(temp, mediaUrl);
        }
        else if((ptrHost != NULL) && (ptrPort == NULL))
        {
            strncat(temp, mediaUrl, strlen(mediaUrl) - strlen(ptrHost));
            strcat(temp, host);
            strncat(temp, ptrHost + strlen(HOST), strlen(ptrHost) - strlen(ptrHost + strlen(host)) + 1);
        }
        else if((ptrHost == NULL) && (ptrPort != NULL))
        {
            strncat(temp, mediaUrl, strlen(mediaUrl) - strlen(ptrPort));
            strcat(temp, port);
            strncat(temp, ptrPort + strlen(PORT), strlen(ptrPort) - strlen(ptrPort + strlen(port)) + 1);

        }
        else if((ptrHost != NULL) && (ptrPort != NULL))
        {
            strncat(temp, mediaUrl, strlen(mediaUrl) - strlen(ptrHost));
            strcat(temp, host);
            strncat(temp, ptrHost + strlen(HOST), strlen(ptrHost) - strlen(ptrPort) - strlen(PORT));
            strcat(temp, port);
            strncat(temp, ptrPort + strlen(PORT), strlen(ptrPort) - strlen(ptrPort + strlen(port)) + 1);
        }

        memset(mediaUrl, 0, sizeof(mediaUrl));
        strcpy(mediaUrl, temp);

        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sMEDIA-URL ===> [%s]"), MEDIA, mediaUrl);
        info_log(LOG_GLOBAL_BUFFER);
    }
    else
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sMedia-URL not available."), MEDIA_ERROR);
        error_log(LOG_GLOBAL_BUFFER);
    }
}
