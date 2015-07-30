/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/



#ifndef INSTAMSG_NETWORK
#define INSTAMSG_NETWORK

typedef struct Network Network;
struct Network
{
    void *medium;

    /*
     * The following two "read" and "write" functions must return "FAILURE" """immediately""" on the
     * first error encountered due to error on the underlying communication-medium (eg. socket).
     *
     * When the calling-function receives "FAILURE", it cleans up the communication-medium itself, and re-initializes
     * the communication-medium.
     *
     * Note that in the general case, once the communication-medium (eg. socket) is set up, there will be no
     * read/write errors as such. RARELY (please note the CAPS), when the error does happen (for eg.
     * network-fluctuation, peer closes its end), cleaning and re-initializing is the cleanest way to
     * get back things to good.
     *
     * ALSO PLEASE NOTE THAT THE CLEANING AND RE-INITIALIZATION HAPPENS FROM THE CALLING FUNCTION.
     * THE IMPLEMENTATION """MUST NOT""" DO ANY CLEANUP BEFORE RETURING "FAILURE", ELSE THE APPLICATION
     * WILL DEFINITELY CRASH DUE TO MULTIPLE FREEING OF RESOURCES.
     *
     * Kindly refer to the implementation in "tcp_socket.c" for steps and details.
     */
    int (*read)(Network *network, unsigned char* buffer, int len);
    int (*write)(Network *network, unsigned char* buffer, int len);
};

Network* get_new_network(void *arg);
void release_network(Network*);

#endif
