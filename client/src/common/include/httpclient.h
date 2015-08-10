#include "./instamsg_vendor_common.h"

int downloadFile(const char *url,
                 const char *downloadedFileName,
                 KeyValuePairs *params,
                 KeyValuePairs *headers,
                 unsigned int timeout);


int uploadFile(const char *url,
                 const char *filename,
                 KeyValuePairs *params,
                 KeyValuePairs *headers,
                 unsigned int timeout,
                 unsigned char *urlValue);
