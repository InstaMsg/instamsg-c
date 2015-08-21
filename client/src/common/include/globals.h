#ifndef INSTAMSG_GLOBALS
#define INSTAMSG_GLOBALS

#define MAX_BUFFER_SIZE 1000

enum returnCode
{
    SOCKET_READ_TIMEOUT = -3,
    BUFFER_OVERFLOW = -2,
    FAILURE = -1,
    SUCCESS = 0
};

enum ValueType
{
    STRING = 0,
    INTEGER
};


typedef struct KeyValuePairs KeyValuePairs;
struct KeyValuePairs
{
    char *key;
    char *value;
};


#define HTTP_FILE_DOWNLOAD_SUCCESS 200
#define HTTP_FILE_UPLOAD_SUCCESS 200

#define CONTENT_LENGTH "Content-Length"
#define POST_BOUNDARY "-----------ThIs_Is_tHe_bouNdaRY_78564$!@"

#define SOCKET_READ     "[SOCKET-READ] "
#define SOCKET_WRITE    "[SOCKET-WRITE] "

#define NETWORK_NOT_AVAILABLE "Could not connect to the network at physical-layer ... not retrying"
#define MAX_CONN_ATTEMPTS_WITH_PHYSICAL_LAYER_FINE 5


////////////////////////////////////////////////////////////////////////////////
#define INSTAMSG_HOST       "localhost"
#define INSTAMSG_PORT       1883
#define INSTAMSG_HTTP_HOST  "localhost"
#define INSTAMSG_HTTP_PORT  80
#define LOG_LEVEL           2

#define NETWORK_READ_TIMEOUT_SECS 1
#define MAX_TRIES_ALLOWED_WHILE_READING_FROM_NETWORK_MEDIUM 5
///////////////////////////////////////////////////////////////////////////////

int sg_print(char **out, int *varg);
int sg_sprintf(char *out, const char *format, ...);

#if 0
#define SG_MEMSET(dest, byte, size)                             \
    {                                                           \
        char temp[MAX_BUFFER_SIZE] = {byte};                    \
        memcpy(dest, temp, size);                               \
    }
#endif

#if 1
void DO_SG_MEMSET(char *dest, char byte, unsigned int size);
#define SG_MEMSET(dest, byte, size)     DO_SG_MEMSET(dest, byte, size);
#endif

#endif
