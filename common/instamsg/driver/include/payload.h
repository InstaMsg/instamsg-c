#ifndef IOEYE_PAYLOAD
#define IOEYE_PAYLOAD

#include "device_defines.h"

#define TOPIC_WEBHOOK           "instamsg/webhook"

extern char *stringified_ioeye_data_prefix;

void pre_process_payload();
void post_process_payload(unsigned int errorCase);

#if SEND_GPS_LOCATION == 1
void ioeye_send_gps_data_to_server(const char *data);
#endif

#if SEND_GPIO_INFORMATION == 1
void ioeye_send_gpio_data_to_server(const char *data);
#endif

#endif
