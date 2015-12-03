#ifndef IOEYE_MODBUS_COMMON
#define IOEYE_MODBUS_COMMON

#include "./globals.h"
#include "device_modbus.h"


typedef short (*SHORT_VALUE_GETTER)(void *);

/*
 * Global-functions callable.
 */
void init_modbus(Modbus *modbus, MODBUS_DEVICE_TYPE deviceType, const char *identifier,
                 SHORT_VALUE_GETTER shortPayloadValueGetter, void *shortPayloadValueGetterArg);
void release_modbus(Modbus *modbus);
void modbusOnConnectProcedures(Modbus *modbus);
void modbusProcedures(Modbus *modbus);
void resetSimulatedModbusEnvironment(int numberOfSimulatedInterfaces);
void flushSimulatedModbusEnvironment();


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

char *simulatedModbusValuesCumulated;

#endif
