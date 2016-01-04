#ifndef DEVICE_MEDIA
#define DEVICE_MEDIA

/*
 * This method starts streaming, to the desired media-server.
 * THIS MUST BE DONE IN A DEDICATED THREAD.
 */
void create_and_start_streaming_pipeline(const char *mediaServerIpAddress, const char *mediaServerPort)
{
}


/*
 * This method pauses the streaming.
 */
void pause_streaming()
{
}


/*
 * This method stops the streaming.
 */
void stop_streaming()
{
}
#endif
