#ifndef INSTAMSG_MEM
#define INSTAMSG_MEM

#include "globals.h"
#include "stdint.h"

#define HEADER_SIZE 2

void* DEFAULT_MALLOC(unsigned short numBytes);
void DEFAULT_FREE(void *ptr);
char* sg_mem_strstr(char *string, const char *substring, int len);

#endif
