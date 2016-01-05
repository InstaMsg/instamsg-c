#ifndef INSTAMSG_MEDIA
#define INSTAMSG_MEDIA

void create_and_start_streaming_pipeline(const char *mediaServerIpAddress, const char *mediaServerPort);
void pause_streaming();
void stop_streaming();

#endif
