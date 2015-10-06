#ifndef IOEYE_MODBUS_COMMON
#define IOEYE_MODBUS_COMMON

#include "device_modbus.h"

/*
 * Global-functions callable.
 */
void init_modbus(Modbus *modbus, void *arg);
void release_modbus(Modbus *modbus);


/*
 * Must not be called directly.
 * Instead must be called as ::
 *
 *      modbus->send_command_and_read_response_sync
 */
int modbus_send_command_and_read_response_sync(Modbus *modbus,
                                               unsigned char *commandBytes,
                                               int commandBytesLength,
                                               unsigned char *responseByteBuffer,
                                               int responseBytesLength);


/*
 * Internally Used.
 * Must not be called by anyone.
 */
void connect_underlying_modbus_medium_guaranteed(Modbus *modbus);
void release_underlying_modbus_medium_guaranteed(Modbus *modbus);
unsigned long getExpectedModbusResponseLength(char *commandNibbles);
void fillPrefixIndices(char *commandNibbles, int *prefixStartIndex, int *prefixEndIndex);

#endif
