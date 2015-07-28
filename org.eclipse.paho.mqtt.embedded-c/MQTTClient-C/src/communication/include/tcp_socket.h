#ifndef INSTAMSG_NETWORK
#define INSTAMSG_NETWORK

#include "./common.h"

typedef struct Network Network;
struct Network
{
    COMMUNICATION_FIELDS
};

Network* get_new_network();
void release_network(Network*);

#endif
