#ifndef INSTAMSG_GLOBALS
#define INSTAMSG_GLOBALS

#define MAX_BUFFER_SIZE 1000

enum returnCode
{
   BUFFER_OVERFLOW = -2,
   FAILURE = -1,
   SUCCESS = 0
};

enum ValueType
{
    STRING = 0,
    INTEGER
};


typedef struct NetworkParameters NetworkParameters;
struct NetworkParameters
{
    unsigned char hostName[MAX_BUFFER_SIZE];
    int port;
};


typedef struct KeyValuePairs KeyValuePairs;
struct KeyValuePairs
{
    char *key;
    char *value;
};


#define HTTP_FILE_DOWNLOAD_SUCCESS 200

#endif
