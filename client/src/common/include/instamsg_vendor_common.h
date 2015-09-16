#ifndef INSTAMSG_VENDOR_COMMON
#define INSTAMSG_VENDOR_COMMON


/*
 * The following two "read" and "write" functions must return "FAILURE" """immediately""" on the
 * first error encountered on the underlying communication-medium (eg. socket, file, serial-port, etc).
 *
 * Alternatively, if it is a "read" function and "guaranteed" parameter is 0 (meaning it is a non-blocking function) and
 * the time-expires without any data being read, the "read" function must return SOCKET_READ_TIMEOUT.
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
    int (*read) (type *interface, unsigned char* buffer, int len, unsigned char guaranteed);    \
    int (*write)(type *interface, unsigned char* buffer, int len);


#ifdef FILE_SYSTEM_INTERFACE_ENABLED
/*
 * Interface for sending/receiving bytes between the instamg-client (on the device) and the device-file(system).
 */
#define FILE_SYSTEM_INTERFACE                                                                                       \
    COMMUNICATION_INTERFACE(FileSystem)                                                                             \
                                                                                                                    \
    /*                                                                                                              \
     * This method renames/moves a file, and returns SUCCESS (0) for success, else a non-zero value.                \
     */                                                                                                             \
    int (*renameFile)(FileSystem *fs, const char *oldPath, const char *newPath);                                    \
                                                                                                                    \
                                                                                                                    \
    /*                                                                                                              \
     * This method deletes a file, and returns SUCCESS (0) for success, else a non-zero value.                      \
     */                                                                                                             \
    int (*deleteFile)(FileSystem *fs, const char *filePath);                                                        \
                                                                                                                    \
                                                                                                                    \
    /*                                                                                                              \
     * This method returns the file-listing in the directory specified, and populates the "buf".                    \
     * The format is ::                                                                                             \
     *                                                                                                              \
     *          {"publisher.sh":152,"subscriber.sh":140,"filetester.sh":137,"config.txt_filetester":148,"stdoutsub":56040,"build_ubuntu_14_04.sh":2892,"README.md":1502,"config.txt":128,"stdoutsub.c":6649,"config.txt_local_testing":128}                         \
     */                                                                                                             \
    void (*getFileListing)(FileSystem *fs, char *buf, int maxValueLenAllowed, const char *directoryPath);           \
                                                                                                                    \
                                                                                                                    \
    /*                                                                                                              \
     * This method returns a long-value, specifying the size of file in bytes.                                      \
     */                                                                                                             \
    long (*getFileSize)(FileSystem *fs, const char *filepath);


typedef struct FileSystem FileSystem;
void init_file_system(FileSystem *fs, void *arg);
void release_file_system(FileSystem *fs);
#endif


#endif

