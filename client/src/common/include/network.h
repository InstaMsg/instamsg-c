#ifndef INSTAMSG_NETWORK_COMMON
#define INSTAMSG_NETWORK_COMMON

#include "communication/network.h"

void init_network(Network *network, const char *hostName, unsigned int port);
void release_network(Network *network);


void connect_underlying_medium_try_once(Network* network, char *hostName, int port);
int network_read (Network *network, unsigned char* buffer, int len, unsigned char guaranteed);
int network_write(Network *network, unsigned char* buffer, int len);
void release_underlying_medium_guaranteed(Network* network);


#endif
