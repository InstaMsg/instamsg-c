#include "./instamsg_vendor_common.h"

int downloadFile(Network *network,
                 const char *url,
                 const char *downloadedFileName,
                 KeyValuePairs *params,
                 KeyValuePairs *headers,
                 unsigned int timeout);

