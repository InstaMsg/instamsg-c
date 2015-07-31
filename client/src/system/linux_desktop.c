/*******************************************************************************
 * Contributors:
 *
 *    Ajay Garg <ajay.garg@sensegrow.com>
 *******************************************************************************/


#include <stdio.h>
#include <stdlib.h>

#include "include/system.h"


static void getManufacturer(System *system, unsigned char *buf, int maxValueLenAllowed)
{
    snprintf(buf, maxValueLenAllowed, "HP");
}


static void getSerialNumber(System *system, unsigned char *buf, int maxValueLenAllowed)
{
    snprintf(buf, maxValueLenAllowed, "123456789");
}


System* get_new_system()
{
	System *system = (System*) malloc(sizeof(System));

    system->getManufacturer = getManufacturer;
    system->getSerialNumber = getSerialNumber;

    return system;
}


void release_system(System* system)
{
    if(system != NULL)
    {
	    free(system);
    }
}
