#ifndef IOEYE_METADATA_COMMON
#define IOEYE_METADATA_COMMON

void get_client_session_data(char *messageBuffer, int maxBufferLength);
void get_client_metadata(char *messageBuffer, int maxBufferLength);
void get_network_data(char *messageBuffer, int maxBufferLength);
void get_manufacturer(char *messagebuffer, int maxbufferlength);
void get_serial_number(char *messagebuffer, int maxbufferlength);

#endif
