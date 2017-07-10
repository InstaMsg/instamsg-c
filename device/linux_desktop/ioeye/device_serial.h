#ifndef IOEYE_SERIAL
#define IOEYE_SERIAL

#include "../../../common/instamsg/driver/include/globals.h"
#include "../../../common/instamsg/driver/include/payload.h"

#include "device_socket.h"

typedef struct Serial Serial;
typedef struct SimulatedModbus SimulatedModbus;

struct Serial
{
    /* ============================= THIS SECTION MUST NOT BE TEMPERED ==================================== */
    unsigned char isSimulatedDevice;
    char identifier[50];
    char serial_params_identifier[100];
    char serial_delimiter_identifier[100];
    char port_type_identifier[100];
    unsigned char assignedSerialNumber;
    int (*send_command_and_read_response_sync)(Serial *serial,
                                               unsigned char *commandBytes,
                                               int commandBytesLength,
                                               unsigned char *responseByteBuffer,
                                               int *responseBytesLength);

    char serialCommands[SERIAL_COMMANDS_BUFFER_SIZE];
    char serialParams[50];
    char serialDelimiter[5];
    char portType[50];

    char portName[50];
    char portAddress[50];
    char hostAddress[50];
    char hostPort[50];

    SG_Socket modbusSocket;

    unsigned char commandsLoaded;
    /* ============================= THIS SECTION MUST NOT BE TEMPERED ==================================== */



    /* ============================= ANY EXTRA FIELDS GO HERE ============================================= */
    int fd;
    /* ============================= ANY EXTRA FIELDS GO HERE ============================================= */
};
#endif
