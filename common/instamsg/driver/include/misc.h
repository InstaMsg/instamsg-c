#ifndef INSTAMSG_MISC_COMMON
#define INSTAMSG_MISC_COMMON

/*
 * Need to be implemented by the device.
 */
void resetDevice();
void bootstrapInit();
void get_client_session_data(char *messageBuffer, int maxBufferLength);
void get_client_metadata(char *messageBuffer, int maxBufferLength);
void get_network_data(char *messageBuffer, int maxBufferLength);
void get_manufacturer(char *messagebuffer, int maxbufferlength);
void get_device_uuid(char *buffer, int maxbufferlength);
void get_device_ip_address(char *buffer, int maxbufferlength);


/*
 * Need to be implemented by the app.
 */
void release_app_resources();


/*
 * INTERNAL.
 */
#if GSM_INTERFACE_ENABLED == 1
#else
void get_prov_pin_for_non_gsm_devices(char *buffer, int maxbufferlength);
#endif

void exitApp();

#endif
