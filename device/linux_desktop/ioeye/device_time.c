#include <time.h>
#include <string.h>

#include "../../../common/instamsg/driver/include/globals.h"


/*
 * This method gets the time in the following-format
 *
 *          YYYYMMDD4HHMMSS
 *          201507304155546
 */
void getTimeInDesiredFormat(char *buffer, int maxBufferLength)
{
    time_t rawtime;
    struct tm *info;

    time(&rawtime);
    info = localtime(&rawtime);

    strftime(buffer, maxBufferLength,"%Y%m%d4%H%M%S", info);
}


/*
 * This method gets the timezone-offset for this device.
 */
void getTimezoneOffset(char *buffer, int maxBufferLength)
{
  time_t t = time(NULL);
  struct tm lt = {0};

  localtime_r(&t, &lt);

  sg_sprintf(buffer, "%ld", lt.tm_gmtoff);
}
