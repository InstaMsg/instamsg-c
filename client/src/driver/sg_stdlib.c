#include <string.h>

int sg_atoi(const char *buf)
{
    int result, len, i, exponent, k;
    unsigned char someNumberParsed = 0;

    result = 0;
    exponent = -1;

    len = strlen(buf);
    for(i = len - 1; i >= 0; i--)
    {
        int base = 1;

        if(buf[i] == ' ')
        {
            if(someNumberParsed == 0)
            {
                continue;
            }
            else
            {
                return result;
            }
        }

        someNumberParsed = 1;

        exponent++;
        for(k = 0; k < exponent; k++)
        {
            base = base * 10;
        }

        result = result + (base * (buf[i] - '0'));
    }

    return result;
}
