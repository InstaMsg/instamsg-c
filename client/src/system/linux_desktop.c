/*******************************************************************************
 * Contributors:
 *
 *    Ajay Garg <ajay.garg@sensegrow.com>
 *******************************************************************************/


#include <stdio.h>
#include <stdlib.h>

#include "instamsg_vendor.h"

static void getManufacturer(System *system, unsigned char *buf, int maxValueLenAllowed)
{
    snprintf(buf, maxValueLenAllowed, "HP");
}


static void getSerialNumber(System *system, unsigned char *buf, int maxValueLenAllowed)
{
    snprintf(buf, maxValueLenAllowed, "123456789");
}


void init_system_utils(System *system)
{
    system->getManufacturer = getManufacturer;
    system->getSerialNumber = getSerialNumber;
}


void release_system_utils(System *system)
{
}
