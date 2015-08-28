#ifndef INSTAMSG_MEM
#define INSTAMSG_MEM

#include "globals.h"

#define HEADER_SIZE 2

char sg_heap[MAX_HEAP_SIZE];

void* sg_malloc(unsigned short numBytes);
void sg_free(void *ptr);

#endif
