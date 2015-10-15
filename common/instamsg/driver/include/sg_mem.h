#ifndef INSTAMSG_MEM
#define INSTAMSG_MEM

#include "globals.h"
#include "stdint.h"

#define HEADER_SIZE 2

char sg_heap[MAX_HEAP_SIZE];

void* sg_malloc(unsigned short numBytes);
void sg_free(void *ptr);
char* sg_mem_strstr(char *string, const char *substring, int len);

#endif
