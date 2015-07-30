#ifndef INSTAMSG_GLOBALS
#define INSTAMSG_GLOBALS

#define MAX_BUFFER_SIZE 100

enum returnCode
{
   BUFFER_OVERFLOW = -2,
   FAILURE = -1,
   SUCCESS = 0
};

typedef enum ValueType ValueType;
enum ValueType
{
    STRING = 0,
    INTEGER
};


#endif
