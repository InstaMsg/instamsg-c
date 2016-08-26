#ifndef IOEYE_SERIAL
#define IOEYE_SERIAL

#include "../../../common/instamsg/driver/include/globals.h"
#include "../../../common/ioeye/include/globals.h"

typedef struct Serial Serial;

struct Serial
{
    /* ============================= THIS SECTION MUST NOT BE TEMPERED ==================================== */
    SERIAL_DEVICE_TYPE deviceType;
    const char *identifier;
    unsigned char assignedSerialNumber;
    int (*send_command_and_read_response_sync)(Serial *serial,
                                               unsigned char *commandBytes,
                                               int commandBytesLength,
                                               unsigned char *responseByteBuffer,
                                               int *responseBytesLength,
                                               unsigned char delimiter);

    char serialCommands[SERIAL_COMMANDS_BUFFER_SIZE];
    unsigned char commandsLoaded;
    /* ============================= THIS SECTION MUST NOT BE TEMPERED ==================================== */



    /* ============================= ANY EXTRA FIELDS GO HERE ============================================= */
    /* ============================= ANY EXTRA FIELDS GO HERE ============================================= */
};
#endif
