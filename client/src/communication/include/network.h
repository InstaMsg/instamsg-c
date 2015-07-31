/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

/*
 * Interface for sending/receiving bytes between the device and instamsg-server.
 */

#ifndef INSTAMSG_NETWORK
#define INSTAMSG_NETWORK

#include "./common.h"

typedef struct Network Network;
struct Network
{
    COMMUNICATION_INTERFACE(Network)
};

Network* get_new_network(void *arg);
void release_network(Network*);

#endif
