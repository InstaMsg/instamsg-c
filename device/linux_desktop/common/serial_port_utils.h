#ifndef COMMON_SERIAL_PORT_UTILS
#define COMMON_SERIAL_PORT_UTILS

void connect_serial_port(int *fd, const char *port_name, int speed, int parity, int chars, int blocking);
int disconnect_serial_port(int fd);

#endif
