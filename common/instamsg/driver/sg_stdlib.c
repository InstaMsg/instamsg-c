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


char* sg_strnstr(char *str1, char *str2, int maxSize)
{
    int i = 0, j = 0;

    if(str1 == NULL)
    {
        return NULL;
    }
    if(str2 == NULL)
    {
        return NULL;
    }

    for(i = 0; i < (maxSize - strlen(str2)); i++)
    {
        unsigned char failure = 0;
        for(j = 0; j < strlen(str2); j++)
        {
            if(str1[i + j] != str2[j])
            {
                failure = 1;
                break;
            }
        }

        if(failure == 0)
        {
            return (str1 + i);
        }
    }

    return NULL;
}


static char temp[100];
void get_nth_token(char *original, char *separator, int pos, char **res)
{
    int i;

    memset(temp, 0, sizeof(temp));
    memcpy(temp, original, strlen(original));

    *res = strtok(temp, separator);
    if(*res == NULL)
    {
        return;
    }

    for(i = 1; i < pos; i++)
    {
        *res = strtok(NULL, separator);
    }
}
