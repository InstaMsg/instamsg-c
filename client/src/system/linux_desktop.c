/*******************************************************************************
 * Contributors:
 *
 *    Ajay Garg <ajay.garg@sensegrow.com>
 *******************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>

#include "instamsg_vendor.h"

static void getManufacturer(System *system, unsigned char *buf, int maxValueLenAllowed)
{
    snprintf(buf, maxValueLenAllowed, "HP");
}


static void getSerialNumber(System *system, unsigned char *buf, int maxValueLenAllowed)
{
    snprintf(buf, maxValueLenAllowed, "123456789");
}


static void getFileListing(System *system, unsigned char *buf, int maxValueLenAllowed, const char *directoryPath)
{
    int len;
    struct dirent *pDirent;
    DIR *pDir;

    pDir = opendir(directoryPath);
    if(pDir == NULL)
    {
        error_log("Cannot open directory '%s'\n", directoryPath);
        return;
    }

    char firstEntryDone = 0;

    strcat(buf, "{");
    while ((pDirent = readdir(pDir)) != NULL)
    {
        struct stat path_stat;
        stat(pDirent->d_name, &path_stat);

        if(S_ISREG(path_stat.st_mode))
        {
            char newEntry[MAX_BUFFER_SIZE] = {0};
            sprintf(newEntry, "\"%s\":%ld", pDirent->d_name, (long)path_stat.st_size);

            if((strlen(buf) + strlen(newEntry)) < (maxValueLenAllowed - 10))
            {
                if(firstEntryDone == 1)
                {
                    strcat(buf, ",");
                }
                strcat(buf, newEntry);
                firstEntryDone = 1;
            }
            else
            {
                break;
            }
        }
    }
    strcat(buf, "}");
    closedir(pDir);
}


static void rebootDevice(System *sys)
{
    info_log("Rebooting the system.");
    system("/sbin/reboot");
}


void init_system_utils(System *system, void *arg)
{
    system->getManufacturer = getManufacturer;
    system->getSerialNumber = getSerialNumber;
    system->getFileListing = getFileListing;
    system->rebootDevice = rebootDevice;
}


void release_system_utils(System *system)
{
}
