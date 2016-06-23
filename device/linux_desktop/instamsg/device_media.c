#ifndef DEVICE_MEDIA
#define DEVICE_MEDIA

#include "../../../common/instamsg/driver/include/globals.h"
#include "../../../common/instamsg/driver/include/media.h"

#if MEDIA_STREAMING_ENABLED == 1
#include <string.h>

#include <gst/gst.h>


char mediaIp[20];
char mediaPort[20];

static void *startPipeline(void *arg)
{
    GstElement *pipeline;
    GstBus *bus;
    GstMessage *msg;

    gst_init (NULL, NULL);

    if(strlen(mediaUrl) == 0)
    {
        return;
    }

    pipeline = gst_parse_launch (mediaUrl, NULL);
    gst_element_set_state (pipeline, GST_STATE_PLAYING);


    /*
     * Wait until error or EOS.
     */
    bus = gst_element_get_bus (pipeline);
    msg = gst_bus_timed_pop_filtered (bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);

    /*
     * If code reaches here, it means the media-loop had some error occurred.
     */
    mediaStreamingErrorOccurred = 1;

    /*
     * Free resources
     */
    if (msg != NULL)
        gst_message_unref (msg);

    gst_object_unref (bus);
    gst_element_set_state (pipeline, GST_STATE_NULL);
    gst_object_unref (pipeline);
}


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
static pthread_t tid;
void create_and_start_streaming_pipeline(const char *mediaServerIpAddress, const char *mediaServerPort)
{
    memset(mediaIp, 0, sizeof(mediaIp));
    strcpy(mediaIp, mediaServerIpAddress);

    memset(mediaPort, 0, sizeof(mediaPort));
    strcpy(mediaPort, mediaServerPort);

    processGstTypeUrl(mediaIp, mediaPort);
    pthread_create(&tid, NULL, startPipeline, NULL);
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
