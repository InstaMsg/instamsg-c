#ifndef INSTAMSG_HTTPCLIENT
#define INSTAMSG_HTTPCLIENT

#include "./globals.h"

typedef struct HTTPResponse HTTPResponse;
struct HTTPResponse
{
    int status;
    char body[MAX_BUFFER_SIZE];
};


HTTPResponse downloadFile(const char *url,
                          const char *downloadedFileName,
                          KeyValuePairs *params,
                          KeyValuePairs *headers,
                          unsigned int timeout);


#if FILE_SYSTEM_ENABLED == 1
HTTPResponse uploadFile(const char *url,
                        const char *filename,
                        KeyValuePairs *params,
                        KeyValuePairs *headers,
                        unsigned int timeout);

#endif
#endif
