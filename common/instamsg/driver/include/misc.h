#ifndef INSTAMSG_MISC_COMMON
#define INSTAMSG_MISC_COMMON

void rebootDevice();
void bootstrapInit();
void get_client_session_data(char *messageBuffer, int maxBufferLength);
void get_client_metadata(char *messageBuffer, int maxBufferLength);
void get_network_data(char *messageBuffer, int maxBufferLength);
void get_manufacturer(char *messagebuffer, int maxbufferlength);
void get_device_uuid(char *buffer, int maxbufferlength);
void get_device_ip_address(char *buffer, int maxbufferlength);

#endif
