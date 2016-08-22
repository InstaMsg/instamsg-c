#ifndef COMMON_SERIAL_PORT_UTILS
#define COMMON_SERIAL_PORT_UTILS

void connect_serial_port(int *fd,
                         const char *port_name,
                         int speed,
                         int parity,
                         int odd_parity,
                         int chars,
                         int blocking,
                         int two_stop_bits,
                         int hardware_control);

int disconnect_serial_port(int fd);

#endif
