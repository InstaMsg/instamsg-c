#ifndef DEVICE_DEFINES
#define DEVICE_DEFINES

#define DEVICE_VERSION "1.0.0"

/*
 * Device-implementation of sprintf (https://www.tutorialspoint.com/c_standard_library/c_function_sprintf.htm) function.
 * If the device has no in-built implementation, then the default instamsg-implementation DEFAULT_SPRINTF can be safely used.
 */
#define sg_sprintf                                              DEFAULT_SPRINTF


/*
 * Whether to use instamsg-implementation of malloc or not.
 *
 * If we use instamsg-malloc-implementation, then we are done.
 * Else, set "sg_malloc" and "sg_free" to device-implementations of "malloc" and "free" respectively.
 */
#define USE_DEFAULT_MALLOC                                      1
#if USE_DEFAULT_MALLOC == 1
#define sg_malloc                                               DEFAULT_MALLOC
#define sg_free                                                 DEFAULT_FREE
#else
#define sg_malloc
#define sg_free
#endif


/*
 * Any wrapper for string-literals.
 *
 * Need for this variable first arose when porting instamsg on AVR-boards, as by default all strings are stored in RAM
 * (whereas expectation is that all string literals would be stored in ROM).
 *
 * So, on AVR-architectures, string "instamsg" would normally be stored in RAM, but
 *                                  PSTR("instamsg") would be stored in ROM/Flash.
 *
 * So, in this case, simply do
 * #define PROSTR   PSTR *
 */
#define PROSTR


/*
 * Maximum size of any buffer in any instamsg.
 */
#define MAX_BUFFER_SIZE                                         1000


/*
 * Maximum size of network-read-buffer, when running in an OpenSSL-environment.
 */
#define READ_BUFFER_SIZE                                        MAX_BUFFER_SIZE


/*
 * Determines whether data is to be logged.
 *
 * If this is 1, any message that is failed to be sent to instamsg-server, is logged, and tried later for re-delivery.
 * Else, a failed message is lost forever.
 */
#define DATA_LOGGING_ENABLED                                    1


/*
 * Maximum number of serial-ports in this device.
 */
#define MAX_PORTS_ALLOWED                                       0


/*
 * Whether diagnostic-logging is to be on a filesystem.
 *
 * Setting this to 1 makes sense only when FILE_SYSTEM_ENABLED is also 1.
 */
#define FILE_LOGGING_ENABLED                                    0


/*
 * Whether the device has a fileystem.
 */
#define FILE_SYSTEM_ENABLED                                     0


/*
 * Whether the device is a GSM/GPRS device.
 */
#define GSM_INTERFACE_ENABLED                                   0


/*
 * Whether OpenSSL is enabled.
 */
#define SSL_ENABLED                                             0
#if SSL_ENABLED == 1
#define SSL_WIRE_BUFFER_SIZE                                    MAX_BUFFER_SIZE
#endif



/*
 * Whether socket-level SSL is enabled.
 *
 * At time of documenting this, this variable is set to 1 only in systems where GSM_INTERFACE_ENABLED is 1, and AT-commands
 * support SSL-sockets (eg. SIMCOM800C).
 */
#define SOCKET_SSL_ENABLED                                      0


/*
 * Whether the device has a GPS-interface for time-syncing.
 */
#define GPS_TIME_SYNC_PRESENT                                   0


/*
 * Whether the time has to be synced via GSM (via unsolicited GSM-responses).
 */
#define GSM_TIME_SYNC_PRESENT                                   0


/*
 * If either GSM/GPS time-syncing is present, DEFAULT_MAX_SECONDS_FOR_GPS_GSM_TIME_SYNC defines the default interval
 * for which instamsg tries syncing time via GPS/GSM.
 *
 * The configurable-variable MAX_SECONDS_FOR_GPS_GSM_TIME_SYNC actually controls the maximum-time.
 * Default value of MAX_SECONDS_FOR_GPS_GSM_TIME_SYNC is DEFAULT_MAX_SECONDS_FOR_GPS_GSM_TIME_SYNC.
 */
#if (GPS_TIME_SYNC_PRESENT == 1) || (GSM_TIME_SYNC_PRESENT == 1)
#define DEFAULT_MAX_SECONDS_FOR_GPS_GSM_TIME_SYNC               "900"
#endif


/*
 * Following two macros define if any thread-level locking is required anywhere.
 */
#define ACQUIRE_THREAD_MUTEX
#define RELEASE_THREAD_MUTEX

/*
 * Following two macros define is any locking is required for the thread doing diagnsotic-logging.
 */
#define ACQUIRE_LOG_MUTEX
#define RELEASE_LOG_MUTEX

/*
 * Maximum number of message-handlers active at any moment.
 * Not recommended to be changed.
 */
#define MAX_MESSAGE_HANDLERS                                    5


/*
 * Whether AT-interface is enabled.
 * Makes sense only if GSM_INTERFACE_ENABLED is 1.
 */
#define AT_INTERFACE_ENABLED                                    0


/*
 * This variable defines after how many message-publishes (which equal to socket-writes) is a socket-read done.
 * This is to ensure that the socket-read buffers never reach an overrun limit.
 *
 * 3 is a good default value.
 */
#define DEFAULT_COMPULSORY_SOCKET_READ_AFTER_WRITE_TIMEOUT      "3"


/*
 * Maximum-buffer size holding comma-separated commands (modbus-rtu, modbus-tcp, etc).
 */
#define SERIAL_COMMANDS_BUFFER_SIZE                             600


/*
 * Maximum time to wait for a response to a serial-command.
 *
 * If 0 bytes are received within this interval, then the command-response is marked as "no-response", and
 * same sent to instamsg-server.
 *
 * If complete response is received within this interval, we are done.
 *
 * If partial response is received within this interval, device resets.
 */
#define SERIAL_RESPONSE_TIMEOUT_SECS                            10


/*
 * Keep this as 0.
 */
#define ENSURE_EXPLICIT_TIME_SYNC                               0


/*
 * "Almost" equals to the number of seconds to wait for an acknowledgement from instamsg-server for a published message.
 * If no PUBACK comes, then
 *
 *      i) The message is logged for later re-delivery (provided DATA_LOGGING_ENABLED is 1), AND
 *      ii) Device resets after completion of the current business-logic loop.
 */
#define MAX_CYCLES_TO_WAIT_FOR_PUBACK                           20


/*
 * a) See code for usage of GPS_LOCATION_SENTENCE_TYPE
 *
 * b) DEFAULT_SEND_GPS_LOCATION_INTERVAL determines interval after which a message, dedicatedly containing GPS-information, is sent.
 *    If DEFAULT_SEND_GPS_LOCATION_INTERVAL is 0, then this dedicated message is never sent ("normal" messages, containing location
 *    as a geo-tag, will be continued to be sent as usual).
 */
#define SEND_GPS_LOCATION                                       0
#if SEND_GPS_LOCATION == 1
#define GPS_LOCATION_SENTENCE_TYPE                              ""
#define DEFAULT_SEND_GPS_LOCATION_INTERVAL                      "0"
#endif


/*
 * CRON_ENABLED determines whether CROM-feature is to be enabled.
 *
 * If it is 1, then DEFAULT_CRON_CONFIG determines the default value of config CRON
 */
#define CRON_ENABLED                                            0
#if CRON_ENABLED == 1
#define DEFAULT_CRON_CONFIG                                     ""
#endif


/*
 * SEND_GPIO_INFORMATION determines whether the GPIO-functionality is to be enabled.
 *
 * If it is 1, DEFAULT_GPIO_PINS_ORIENTATION determines default value of config GPIO_PINS_ORIENTATION
 *            GPIO_PIN_ORIENTATION_CONFIG_LENGTH determines the maximum length of the possible values of config GPIO_PINS_ORIENTATION
 */
#define SEND_GPIO_INFORMATION                                   0
#if SEND_GPIO_INFORMATION == 1
#define DEFAULT_GPIO_PINS_ORIENTATION                           ""
#define GPIO_PIN_ORIENTATION_CONFIG_LENGTH                      200
#endif


/*
 * If COMPULSORY_NTP_SYNC is 1, then the device is ALWAYS (tried to be) synced via NTP, even if it has been synced earlier
 * via other resources (GPS/GSM).
 *
 * Generally, if the device supports at least GPS or GSM time-syncing too, then it does not matter what source causes
 * time-syncing, and so COMPULSORY_NTP_SYNC can be 0.
 *
 * But let's take example of the following system with following-characteristics ::
 *
 *          * Native-SOC with a modem and RTC.
 *          * GSM-time syncing is present, which syncs the modem-time, but NOT the RTC time.
 *          * Device needs synced time on RTC, so we ought to run NTP-syncing (which would then also sync RTC-time).
 */
#define COMPULSORY_NTP_SYNC                                     1


/*
 * Determines whether time should be (tried to be) synced via NTP.
 * No reason to not let this variable be 1.
 */
#define NTP_TIME_SYNC_PRESENT                                   1


/*
 * DEFAULT_NTP_SERVER determines the default of config NTP_SERVER
 */
#define DEFAULT_NTP_SERVER                                      "pool.ntp.org"


/*
 * Some devices need server-name as DNS, some as IP.
 * So, this variable has been pulled in this device-section.
 */
#define INSTAMSG_HOST                                           "device.instamsg.io"


#endif
