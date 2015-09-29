#ifndef IOEYE_METADATA_COMMON
#define IOEYE_METADATA_COMMON

void get_client_session_data(char *messageBuffer, int maxBufferLength);
void get_client_metadata(char *messageBuffer, int maxBufferLength);
void get_network_data(char *messageBuffer, int maxBufferLength);

#endif
