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


/*
 * Interface for sending/receiving bytes between the device and instamsg-server.
 */
typedef struct Network Network;
void init_network(Network *network, const char *hostName, unsigned int port);
void release_network(Network *network);


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
    void (*getFileListing)(FileSystem *fs, unsigned char *buf, int maxValueLenAllowed, const char *directoryPath);  \
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


/*
 * Interface for sending/receiving bytes over a serial-interface (eg. UART).
 */
typedef struct SerialLoggerInterface SerialLoggerInterface;
void init_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface, void *arg);
void release_serial_logger_interface(SerialLoggerInterface *serialLoggerInterface);


/*
 * Interface for sending/receiving bytes between the instamg-client (on the device) and the device-command-interface.
 *
 * Generally, the interface would be a serial-port, through which the instamsg-client can send the command-bytes, and
 * receive the command-response-bytes.
 */
typedef struct ModbusCommandInterface ModbusCommandInterface;
void init_modbus_command_interface(ModbusCommandInterface *modbusCommandInterface, void *arg);
void release_modbus_command_interface(ModbusCommandInterface *modbusCommandInterface);


/*
 * Timer-Interface
 */
#define TIMER_INTERFACE                                                                                             \
    /*                                                                                                              \
     * This method gets the device-time in the required format.                                                     \
     *                                                                                                              \
     * For example, if the time is 3:55:46 PM on 30th July 2015, then the "buf" must be                             \
     * written with ::                                                                                              \
     *                                                                                                              \
     *          201507304155546                                                                                     \
     *                                                                                                              \
     * Note that the 4 in the 9th place is hardcoded (a requirement for successful-processing at                    \
     * the instamsg-server).                                                                                        \
     *                                                                                                              \
     * Also, note that the "buf" will be all-0-initialized from the callee, so the vendor-implementation            \
     * does not need to bother about that.                                                                          \
     */                                                                                                             \
    void (*getTimeIn_YYYYmmdd4HHMMSS)(Timer *timer, char *buf);                                                     \
                                                                                                                    \
                                                                                                                    \
    /*                                                                                                              \
     * This method returns the timezone-offset of the device in seconds.                                            \
     *                                                                                                              \
     * For example, if the device is deployed in India, then the offset is 5 hours 30 minutes, which                \
     * translates to (5.5 * 60 * 60 = ) 19800 seconds, and the same must be written to "buf".                       \
     *                                                                                                              \
     * Also, note that the "buf" will be all-0-initialized from the callee, so the vendor-implementation            \
     * does not need to bother about that.                                                                          \
     */                                                                                                             \
    void (*getOffset)(Timer *timer, char *buf);                                                                     \
                                                                                                                    \
                                                                                                                    \
    unsigned int (*getMinimumDelayPossibleInMicroSeconds)(Timer *timer);                                            \
                                                                                                                    \
                                                                                                                    \
    void (*minimumDelay)(Timer *timer);


typedef struct Timer Timer;
void init_timer(Timer *timer, void *arg);
void release_timer(Timer *timer);



/*
 * System-Utilities Interface
 */
#define SYSTEM_INTERFACE                                                                                            \
    /*                                                                                                              \
     * Note that the "buf" will be all-0-initialized from the callee, so the vendor-implementation                  \
     * does not need to bother about that.                                                                          \
     */                                                                                                             \
    void (*getManufacturer)(System *system, char *buf, int maxValueLenAllowed);                                     \
                                                                                                                    \
                                                                                                                    \
    /*                                                                                                              \
     * Note that the "buf" will be all-0-initialized from the callee, so the vendor-implementation                  \
     * does not need to bother about that.                                                                          \
     */                                                                                                             \
    void (*getSerialNumber)(System *system, char *buf, int maxValueLenAllowed);                                     \
                                                                                                                    \
                                                                                                                    \
    /*                                                                                                              \
     * Reboots the device.                                                                                          \
     */                                                                                                             \
    void (*rebootDevice)(System *system);                                                                           \
                                                                                                                    \

typedef struct System System;
void init_system_utils(System *system, void *arg);
void release_system_utils(System *system);
void SYSTEM_GLOBAL_INIT();


#endif

