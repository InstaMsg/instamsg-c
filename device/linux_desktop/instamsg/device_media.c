#ifndef DEVICE_MEDIA
#define DEVICE_MEDIA

#include "../../../common/instamsg/driver/include/globals.h"

#include <string.h>

#include <gst/gst.h>



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
    GstElement *pipeline;

    gst_init (NULL, NULL);

    RESET_GLOBAL_BUFFER;
    sg_sprintf((char*) GLOBAL_BUFFER, "v4l2src device=/dev/video0 ! udpsink host=%s port=%s", mediaServerIpAddress, mediaServerPort);

    pipeline = gst_parse_launch ((char*) GLOBAL_BUFFER, NULL);
    gst_element_set_state (pipeline, GST_STATE_PLAYING);
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
