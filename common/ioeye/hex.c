#include <string.h>

int getIntValueOfHexChar(char c)
{
    if((c == 'a') || (c == 'A'))
    {
        return 10;
    }

    if((c == 'b') || (c == 'B'))
    {
        return 11;
    }

    if((c == 'c') || (c == 'C'))
    {
        return 12;
    }

    if((c == 'd') || (c == 'D'))
    {
        return 13;
    }

    if((c == 'e') || (c == 'E'))
    {
        return 14;
    }

    if((c == 'f') || (c == 'F'))
    {
        return 15;
    }

    return (c - 48);
}


void getByteStreamFromHexString(const char *hexString, unsigned char *buffer)
{
    int i, j;

    for(i = 0, j = 0; i < strlen(hexString); i = i + 2, j++)
    {
        int ascii;

        char buf[3] = {0};
        memcpy(buf, hexString + i, 2);


        ascii = 0;
        ascii = ascii + (16 * getIntValueOfHexChar(buf[0]));
        ascii = ascii + getIntValueOfHexChar(buf[1]);

        buffer[j] = ascii;
    }
}

