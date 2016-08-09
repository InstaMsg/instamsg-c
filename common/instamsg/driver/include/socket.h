#ifndef INSTAMSG_SOCKET_COMMON
#define INSTAMSG_SOCKET_COMMON

#include "device_socket.h"

#define SOCKET			PROSTR("[SOCKET] ")
#define SOCKET_ERROR	PROSTR("[SOCKET_ERROR] ")
#define SOCKET_CONNECTION_SOLITARY_ATTEMPT_MAX_ALLOWED_TIME_SECONDS 300

#define SOCKET_TCP      PROSTR("TCP")
#define SOCKET_UDP      PROSTR("UDP")

#define SG_APN          PROSTR("sg_apn")
#define SG_USER         PROSTR("sg_user")
#define SG_PASS         PROSTR("sg_pass")
#define SG_PIN          PROSTR("sg_pin")
#define PROV_PIN        PROSTR("prov_pin")


/*
 * Global-functions callable.
 */
void init_socket(Socket *socket, const char *hostName, unsigned int port, const char *type);
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
#if GSM_INTERFACE_ENABLED == 1
void store_sms_in_config(char *sms, char *smsConfigBuffer, int smsConfigBufferLength);
void get_latest_sms_containing_substring(Socket *socket, char *buffer, const char *prefix);
#endif
void connect_underlying_socket_medium_try_once(Socket* socket);
void release_underlying_socket_medium_guaranteed(Socket* socket);

#endif
