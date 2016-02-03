#ifndef INSTAMSG_MEDIA
#define INSTAMSG_MEDIA

#include "./globals.h"

#define MEDIA           PROSTR("[MEDIA] ")
#define MEDIA_ERROR     PROSTR("[MEDIA-ERROR] ")

void create_and_start_streaming_pipeline(const char *mediaServerIpAddress, const char *mediaServerPort);
void pause_streaming();
void stop_streaming();

char mediaUrl[MAX_BUFFER_SIZE];

void processGstTypeUrl(char *host, char *port);
#endif
