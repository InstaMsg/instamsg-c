#ifndef INSTAMSG_SERIAL_LOGGER
#define INSTAMSG_SERIAL_LOGGER

typedef struct SerialLoggerInterface SerialLoggerInterface;

struct SerialLoggerInterface
{
    /*
     * Any extra fields may be added here.
     */

    int (*write)(SerialLoggerInterface *serialLoggerInterface, unsigned char* buffer, int len);
};

#endif
