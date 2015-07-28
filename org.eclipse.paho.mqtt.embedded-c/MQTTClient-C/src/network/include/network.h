#ifndef INSTAMSG_NETWORK
#define INSTAMSG_NETWORK

typedef struct Network Network;
struct Network
{
        void *physical_medium;

        /*
         * While reading a packet, we do not know before hand how many bytes we wish to
         * read to read a complete packet (this happens in all TLV-based packet-formats.
         *
         * Also, it is the case that an entire packet must be received using the same underlying
         * medium (we don't want to be receiving the Tag/Length part using one medium-instance,
         * and the Payload part using another medium-instance.
         *
         * So, we take "ensure_guarantee" as the variable, and it will be generally true while
         * reading the Tag/Length part. Once the Tag/Length is read, we must read the Payload part
         * too using the SAME underlying medium.
         *
         * If the underlying medium breaks/distorts after the Tag/Length is read but before the Payload
         * could be read, we MUST mark the packet-reading part as a failure on the whole.
         */
        int (*read)(Network* n, unsigned char* buffer, int len, char ensure_guarantee);


        /*
         * While writing a packet, we definitely know "how many" bytes we wish to write in
         * order to send a complete packet.
         *
         * So, we can ensure the guarantee in the underlying implementation itself, and
         * ensure that the function returns only after ALL the bytes are written using the
         * SAME underlying medium (Socket, GPRS, etc).
         */
        void (*write_guaranteed)(Network* n, unsigned char* buffer, int len);
};

Network* get_new_network();
void release_network(Network*);

#endif
