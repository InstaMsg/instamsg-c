#include <string.h>

void trim_buffer_to_contain_only_first_GPRMC_sentence(unsigned char *buffer, int bufferLength)
{
    memset(buffer, 0, bufferLength);
}
