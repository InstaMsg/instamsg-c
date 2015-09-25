#ifndef INSTAMSG_NETWORK_COMMON
#define INSTAMSG_NETWORK_COMMON

#include "device_network.h"

/*
 * Global-functions callable.
 */
void init_network(Network *network, const char *hostName, unsigned int port);
void release_underlying_network_medium_guaranteed(Network* network);


/*
 * Internally Used.
 * Must not be called by anyone.
 */
void release_network(Network *network);
#ifdef GSM_INTERFACE_ENABLED
void get_latest_sms_containing_substring(Network *network, char *buffer, const char *prefix);
#endif
void get_device_uuid(Network *network, char *buffer);
void connect_underlying_network_medium_try_once(Network* network);


/*
 * Must not be called directly.
 * Instead must be called as ::
 *
 *      network->read
 *      network->write
 */
int network_read (Network *network, unsigned char* buffer, int len, unsigned char guaranteed);
int network_write(Network *network, unsigned char* buffer, int len);


#endif
