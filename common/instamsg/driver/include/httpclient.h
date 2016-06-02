#ifndef INSTAMSG_HTTPCLIENT
#define INSTAMSG_HTTPCLIENT

#include "./globals.h"

typedef struct HTTPResponse HTTPResponse;
struct HTTPResponse
{
    int status;
    char body[MAX_BUFFER_SIZE];
};

extern HTTPResponse httpResponse;
#define RESET_HTTP_RESPONSE memset(&httpResponse, 0, sizeof(HTTPResponse));

void downloadFile(const char *url,
                  const char *downloadedFileName,
                  KeyValuePairs *params,
                  KeyValuePairs *headers,
                  unsigned int timeout,
                  HTTPResponse *httpResponse);


#if FILE_SYSTEM_ENABLED == 1
void uploadFile(const char *url,
                const char *filename,
                KeyValuePairs *params,
                KeyValuePairs *headers,
                unsigned int timeout,
                HTTPResponse *httpResponse);

#endif
#endif
