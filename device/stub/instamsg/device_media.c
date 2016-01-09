#ifndef DEVICE_MEDIA
#define DEVICE_MEDIA

#include "../../../common/instamsg/driver/include/globals.h"

#if MEDIA_STREAMING_ENABLED == 1
/*
 * This method starts streaming, to the desired media-server.
 * THIS MUST BE DONE IN A DEDICATED THREAD.
 *
 * If an error occurs while streaming, the variable
 *              mediaStreamingErrorOccurred
 * must be set equal to 1.
 *
 * The variable "mediaStreamingErrorOccurred" is present in "globals.h" (already included in this module by default).
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

#endif
