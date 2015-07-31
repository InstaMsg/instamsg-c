#ifndef INSTAMSG_TIME
#define INSTAMSG_TIME

typedef struct Timer Timer;
struct Timer
{
    void *obj;

    /*
     * This method gets the device-time in the required format.
     *
     * For example, if the time is 3:55:46 PM on 30th July 2015, then the "buf" must be
     * written with ::
     *
     *          201507304155546
     *
     * Note that the 4 in the 9th place is hardcoded (a requirement for successful-processing at
     * the instamsg-server).
     *
     * Also, note that the "buf" will be all-0-initialized from the callee, so the vendor-implementation
     * does not need to bother about that.
     */
    void (*getTimeIn_YYYYmmdd4HHMMSS)(Timer *timer, unsigned char *buf, int maxValueLenAllowed);


    /*
     * This method returns the timezone-offset of the device in seconds.
     *
     * For example, if the device is deployed in India, then the offset is 5 hours 30 minutes, which
     * translates to (5.5 * 60 * 60 = ) 19800 seconds, and the same must be written to "buf".
     *
     * Also, note that the "buf" will be all-0-initialized from the callee, so the vendor-implementation
     * does not need to bother about that.
     */
    void (*getOffset)(Timer *timer, unsigned char *buf, int maxValueLenAllowed);

};

Timer* get_new_timer();
void release_timer(Timer*);

#endif
