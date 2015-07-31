#ifndef INSTAMSG_COMMUNICATION
#define INSTAMSG_COMMUNICATION


/*
 * The following two "read" and "write" functions must return "FAILURE" """immediately""" on the
 * first error encountered on the underlying communication-medium (eg. socket, file, serial-port, etc).
 *
 * When the calling-function receives "FAILURE", it cleans up the communication-medium itself, and re-initializes
 * the communication-medium.
 *
 * Note that in the general case, once the communication-medium is set up, it will continue to behave fine.
 * Thus, for example,
 *
 *      a)
 *      Once a socket is set-up between the peers, the socket will be able to send/receive bytes successfully generally.
 *
 *      b)
 *      Once a file-pointer is opened fine, we can read/write to the file successfully generally.
 *
 *      c)
 *      Once a serial-port is initialized, bytes can be read/written without any issue generally.
 *
 *
 * It is only in exceptional cases, that the error occurs, like for example,
 *
 *      1)
 *      The peer closed the socket (due to network-fluctuations).
 *
 *      2)
 *      File got full, so read failed.
 *
 *      3)
 *      Some electrical problem occured in the serial-port.
 *
 *
 * As is obvious, above are RARE errors, so re-initializing the system in such RARE scenarios is clean and
 * acceptable solution.
 *
 * IF ANY INTERFACE IMPLEMETING THE "read" AND "write" FUNCTIONS IS PRONE TO """FREQUENT""" ERRORS WHILE
 * readING/writING, THEN THAT MUST BE HANDLED SUITABLY BY THE IMPLEMENTATIONS IN THEIR CORRESPONDING CODE.
 * RETURNING "FAILURE" TO THE CALLING FUNCTION WILL RE-INITIALIZE THE SYSTEM.
 *
 * ALSO PLEASE NOTE THAT THE CLEANING AND RE-INITIALIZATION HAPPENS FROM THE CALLING FUNCTION.
 * THE IMPLEMENTATION """MUST NOT""" DO ANY CLEANUP BEFORE RETURING "FAILURE", ELSE THE APPLICATION
 * WILL DEFINITELY CRASH DUE TO MULTIPLE FREEING OF RESOURCES.
 *
 * Kindly refer to the implementation in "tcp_socket.c" and "linux_fs.c" for steps and details.
 */

#define COMMUNICATION_INTERFACE(type)                                                           \
                                                                                                \
    void *medium;                                                                               \
                                                                                                \
    int (*read) (type *interface, unsigned char* buffer, int len);                              \
    int (*write)(type *interface, unsigned char* buffer, int len);

#endif

