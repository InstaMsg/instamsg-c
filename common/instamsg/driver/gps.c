#include <string.h>

#include "device_defines.h"

#include "./include/globals.h"
#include "./include/gps.h"
#include "./include/log.h"
#include "./include/sg_stdlib.h"


void trim_buffer_to_contain_only_first_GPRMC_sentence(unsigned char *buffer, int bufferLength)
{
    unsigned char *original = buffer;
    int i = 0;
    char *ptrBegin = NULL, *ptrEnd = NULL;
    int j = 0;

    /*
     * Discard any 0-bytes in the beginning (as that is seen in live-devices).
     */
    while(1)
    {
        if(i < bufferLength)
        {
            if((*buffer) != 0)
            {
                break;
            }

            buffer++;
            i++;
        }
        else
        {
        	buffer[0] = 0;
        	return;
        }
    }

    /*
     * Now, search for "$GPRMC"
     */
    ptrBegin = sg_strnstr((char*)buffer, "$GPRMC", strlen((char*)buffer) - 1);
    if(ptrBegin != NULL)
    {
        /*
         * Now, search for closing *
         */
        ptrEnd = sg_strnstr(ptrBegin, "*", strlen(ptrBegin) - 1);
        if(ptrEnd != NULL)
        {
            while(1)
            {
                original[j] = *ptrBegin;
                j++;

                if(ptrBegin == ptrEnd)
                {
                    break;
                }
                else
                {
                    ptrBegin++;
                }
            }
        }
    }

    original[j] = 0;

    sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sGPRMC-sentence extracted from NMEA-Blob = [%s]"), GPS, (char*)buffer);
    info_log(LOG_GLOBAL_BUFFER);
}


#if GPS_TIME_SYNC_PRESENT == 1
/*
 *  $GPRMC,220516,A,5133.82,N,00042.24,W,173.8,231.8,130694,004.2,W*70
 *           1    2    3    4    5     6   7    8       9    10  11 12
 *
 *  1   220516     Time Stamp (hhmmss)
 *  2   A          validity - A-ok, V-invalid
 *  3   5133.82    current Latitude
 *  4   N          North/South
 *  5   00042.24   current Longitude
 *  6   W          East/West
 *  7   173.8      Speed in knots
 *  8   231.8      True course
 *  9   130694     Date Stamp (ddmmyy)
 *  10  004.2      Variation
 *  11  W          East/West
 *  12  *70        checksum
 */
int fill_in_time_coordinates_from_GPRMC_sentence(char *buffer, int bufferLength, DateParams *dateParams)
{
    char *original = buffer;
    char *t = NULL;
    unsigned int number;

    /*
     * First do some validations.
     */
    while(1)
    {
        if(strlen(buffer) < 1)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sLength of GPRMC-sentence is less than zero."), GPS_ERROR);
            error_log(LOG_GLOBAL_BUFFER);

            goto failure;
        }

        if(strlen(buffer) > 82)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sLength of GPRMC-sentence [%s] is greater than 82."), GPS_ERROR, original);
            error_log(LOG_GLOBAL_BUFFER);

            goto failure;
        }

        get_nth_token(original, ",", 1, &t);
        if(strcmp(t, "$GPRMC") != 0)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sFirst token is not $GPRMC in GPRMC-sentence [%s]."), GPS_ERROR, original);
            error_log(LOG_GLOBAL_BUFFER);

            goto failure;
        }

        get_nth_token(original, ",", 3, &t);
        if(strcmp(t, "A") != 0)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sThird token [%s] indicates GPRMC-sentence [%s] is invalid."), GPS_ERROR, t, original);
            error_log(LOG_GLOBAL_BUFFER);

            goto failure;
        }

        get_nth_token(original, ",", 2, &t);
        number = sg_atoi(t);
        if( (strlen(t) != 6) || (number < 1) )
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sSecond token [%s] in GPRMC-sentence [%s] is invalid timestamp."), GPS_ERROR, t, original);
            error_log(LOG_GLOBAL_BUFFER);

            goto failure;
        }
        else
        {
            dateParams->tm_sec = number % 100;
            number = number / 100;

            dateParams->tm_min = number % 100;
            number = number / 100;

            dateParams->tm_hour = number;

            get_nth_token(original, ",", 10, &t);
            number = sg_atoi(t);
            if( (strlen(t) != 6) || (number < 1) )
            {
                sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("%sTenth token [%s] in GPRMC-sentence [%s] is invalid datestamp."),
                           GPS_ERROR, t, original);
                error_log(LOG_GLOBAL_BUFFER);

                goto failure;
            }
            else
            {
                dateParams->tm_year = number % 100;
                number = number / 100;

                dateParams->tm_mon= number % 100;
                number = number / 100;

                dateParams->tm_mday = number;

                print_date_info(dateParams, "GPS");
                return SUCCESS;
            }
        }

    }

failure:
    return FAILURE;
}
#endif

