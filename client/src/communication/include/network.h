#ifndef INSTAMSG_NETWORK
#define INSTAMSG_NETWORK

typedef struct Network Network;
struct Network
{
    void *medium;

    int (*read)(Network *network, unsigned char* buffer, int len);
    int (*write)(Network *network, unsigned char* buffer, int len);
};

Network* get_new_network(void *arg);
void release_network(Network*);

#endif
