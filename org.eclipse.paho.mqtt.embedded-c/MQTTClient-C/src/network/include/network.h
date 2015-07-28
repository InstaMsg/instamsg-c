#ifndef INSTAMSG_NETWORK
#define INSTAMSG_NETWORK

typedef struct Network Network;
struct Network
{
        void *physical_medium;

        void (*read)(Network* n, unsigned char* buffer, int len);
        void (*write)(Network* n, unsigned char* buffer, int len);
};

Network* get_new_network();
void release_network(Network*);

#endif
