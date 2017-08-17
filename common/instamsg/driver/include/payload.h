#ifndef IOEYE_PAYLOAD
#define IOEYE_PAYLOAD

#include "device_defines.h"

#define TOPIC_WEBHOOK           "instamsg/webhook"


struct PortInfoArgument
{
    char *portName;
    char *portAddress;
    char *hostAddress;
    char *hostPort;
};


struct CsvMetadataInfo
{
    char *protocol;
    char *fileId;
};


extern char *stringified_ioeye_data_prefix;

void pre_process_payload(void);
void post_process_payload(unsigned int errorCase, void (*func)(char *, void*), void *arg);

void add_port_info(char *buffer, void *arg);
void add_csv_metadata_info(char *buffer, void *arg);

void assignPortInfoToStructure(struct PortInfoArgument *portInfoArgument, char *portName, char *portAddress, char *hostAddress, char *hostPort);
extern struct PortInfoArgument portInfoArgument;

#define PORT_NAME_COM           PROSTR("com")
#define PORT_NAME_WIFI          PROSTR("wifi")
#define PORT_NAME_GPS           PROSTR("gps")
#define PORT_NAME_GPIO          PROSTR("gpio")
#define PORT_NAME_SIMULATED     PROSTR("simulated")

#if SEND_GPS_LOCATION == 1
void ioeye_send_gps_data_to_server(char *data);
#endif

#if SEND_GPIO_INFORMATION == 1
void ioeye_send_gpio_data_to_server(char *data);
#endif

#endif
