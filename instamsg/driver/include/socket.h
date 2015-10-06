#ifndef INSTAMSG_SOCKET_COMMON
#define INSTAMSG_SOCKET_COMMON

#include "device_socket.h"

/*
 * Global-functions callable.
 */
void init_socket(Socket *socket, const char *hostName, unsigned int port);
void release_socket(Socket *socket);


/*
 * Must not be called directly.
 * Instead must be called as ::
 *
 *      socket->read
 *      socket->write
 */
int socket_read(Socket *socket, unsigned char* buffer, int len, unsigned char guaranteed);
int socket_write(Socket *socket, unsigned char* buffer, int len);


/*
 * Internally Used.
 * Must not be called by anyone.
 */
#ifdef GSM_INTERFACE_ENABLED
void get_latest_sms_containing_substring(Socket *socket, char *buffer, const char *prefix);
#endif
void connect_underlying_socket_medium_try_once(Socket* socket);
void release_underlying_socket_medium_guaranteed(Socket* socket);

#endif
