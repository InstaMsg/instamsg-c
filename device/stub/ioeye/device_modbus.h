#ifndef IOEYE_MODBUS
#define IOEYE_MODBUS

typedef struct Modbus Modbus;

struct Modbus
{
    /* ============================= THIS SECTION MUST NOT BE TEMPERED ==================================== */
    int (*modbus_send_command_and_read_response_sync)(Modbus *modbus,
                                                      unsigned char *commandBytes,
                                                      unsigned char *responseByteBuffer,
                                                      int responseBytesLength);
    /* ============================= THIS SECTION MUST NOT BE TEMPERED ==================================== */



    /* ============================= ANY EXTRA FIELDS GO HERE ============================================= */
    /* ============================= ANY EXTRA FIELDS GO HERE ============================================= */
};

#endif
