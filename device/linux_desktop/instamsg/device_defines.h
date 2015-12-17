#ifndef DEVICE_DEFINES
#define DEVICE_DEFINES

#include <stdio.h>

/*
 * We use the vanilla "sprintf" method for Linux.
 */
#define sg_sprintf sprintf

#define MAX_BUFFER_SIZE 1000
#define MAX_HEAP_SIZE (10 * (MAX_BUFFER_SIZE + HEADER_SIZE))

#endif
