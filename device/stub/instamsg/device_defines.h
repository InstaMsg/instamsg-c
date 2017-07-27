#ifndef DEVICE_DEFINES
#define DEVICE_DEFINES

/*
 * By default.. we use the in-built functionality, which uses variable-argument lists.
 *
 * However, this implementation has shown to give problems for Harvard/RISC architectures, so such devices will need
 * to provide the reference to an implementation shipped with their compiler-environment.
 */

#define DEVICE_VERSION "1.0.0"

#define sg_sprintf                                              DEFAULT_SPRINTF

#define USE_DEFAULT_MALLOC                                      1
#define sg_malloc                                               DEFAULT_MALLOC
#define sg_free                                                 DEFAULT_FREE

#define PROSTR

#define MAX_BUFFER_SIZE                                         1000
#define READ_BUFFER_SIZE                                        MAX_BUFFER_SIZE

#define DATA_LOGGING_ENABLED                                    1

#define SERIAL_COMMANDS_BUFFER_SIZE                             600

#define MAX_PORTS_ALLOWED                                       0

#define FILE_LOGGING_ENABLED                                    0
#define FILE_SYSTEM_ENABLED                                     0
#define GSM_INTERFACE_ENABLED                                   0
#define SSL_ENABLED                                             0
#define SOCKET_SSL_ENABLED                                      0

#define GPS_TIME_SYNC_PRESENT                                   0
#define GSM_TIME_SYNC_PRESENT                                   0
#if (GPS_TIME_SYNC_PRESENT == 1) || (GSM_TIME_SYNC_PRESENT == 1)
#define DEFAULT_MAX_SECONDS_FOR_GPS_GSM_TIME_SYNC               "900"
#endif

#define ACQUIRE_THREAD_MUTEX
#define RELEASE_THREAD_MUTEX

#define ACQUIRE_LOG_MUTEX
#define RELEASE_LOG_MUTEX

#define MAX_MESSAGE_HANDLERS                                    5
#define AT_INTERFACE_ENABLED                                    0

#define DEFAULT_COMPULSORY_SOCKET_READ_AFTER_WRITE_TIMEOUT      "3"

#define SERIAL_RESPONSE_TIMEOUT_SECS                            10
#define ENSURE_EXPLICIT_TIME_SYNC                               0
#define MAX_CYCLES_TO_WAIT_FOR_PUBACK                           20

#define SEND_GPS_LOCATION                                       0
#if SEND_GPS_LOCATION == 1
#define GPS_LOCATION_SENTENCE_TYPE                              ""
#define DEFAULT_SEND_GPS_LOCATION_INTERVAL                      "0"
#endif

#define CRON_ENABLED                                            0
#if CRON_ENABLED == 1
#define DEFAULT_CRON_CONFIG                                     ""
#endif

#define SEND_GPIO_INFORMATION                                   0
#if SEND_GPIO_INFORMATION == 1
#define DEFAULT_GPIO_PINS_ORIENTATION                           ""
#define GPIO_PIN_ORIENTATION_CONFIG_LENGTH                      200
#endif

#define COMPULSORY_NTP_SYNC                                     1
#define NTP_TIME_SYNC_PRESENT                                   1
#define DEFAULT_NTP_SERVER                                      "pool.ntp.org"

#define INSTAMSG_HOST                                           "device.instamsg.io"

#endif
