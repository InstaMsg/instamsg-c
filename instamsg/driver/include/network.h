#ifndef INSTAMSG_NETWORK_COMMON
#define INSTAMSG_NETWORK_COMMON

#include "device_network.h"

/*
 * Global-functions callable.
 */
void init_network(Network *network, const char *hostName, unsigned int port);
void release_underlying_network_medium_guaranteed(Network* network);
void get_device_uuid(char *buffer);


/*
 * Must not be called directly.
 * Instead must be called as ::
 *
 *      network->read
 *      network->write
 */
int network_read(Network *network, unsigned char* buffer, int len, unsigned char guaranteed);
int network_write(Network *network, unsigned char* buffer, int len);


/*
 * Internally Used.
 * Must not be called by anyone.
 */
#ifdef GSM_INTERFACE_ENABLED
void get_latest_sms_containing_substring(Network *network, char *buffer, const char *prefix);
#endif
void connect_underlying_network_medium_try_once(Network* network);
void release_network(Network *network);

#endif