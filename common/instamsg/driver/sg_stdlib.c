#include <string.h>

int sg_atoi(const char *buf)
{
    int result, len, i, exponent, k;
    unsigned char someNumberParsed = 0;
    unsigned char negative = 0;

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
                break;
            }
        }
        else if(buf[i] == '-')
        {
            if(someNumberParsed == 0)
            {
            }
            else
            {
                negative = 1;
            }

            continue;
        }
        else if( (buf[i] < '0') || (buf[i] > '9') )
        {
            return 0;
        }

        someNumberParsed = 1;

        exponent++;
        for(k = 0; k < exponent; k++)
        {
            base = base * 10;
        }

        result = result + (base * (buf[i] - '0'));
    }

    if(negative == 1)
    {
        result = result * -1;
    }

    return result;
}


char* sg_strnstr(const char *str1, const char *str2, int maxSize)
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
            return ((char*)(str1 + i));
        }
    }

    return NULL;
}


static char temp[100];
void get_nth_token(char *original, char separator, int pos, char **res)
{
    int buffer_index = 0, num_separators_encountered = 0, token_start_pos = 0, tmp_index = 0;
    int token_end_pos = -1;

    memset(temp, 0, sizeof(temp));

    while(1)
    {
    	if(buffer_index == strlen(original))
    	{
    		break;
    	}

    	if(original[buffer_index] == separator)
    	{
    		num_separators_encountered++;

    		if(num_separators_encountered == pos)
    		{
    			token_end_pos = buffer_index;
    			break;
    		}
    		else
    		{
    			token_start_pos = buffer_index + 1;
    		}
    	}

    	buffer_index++;
    }


    if(token_end_pos == -1)
    {
    	if(num_separators_encountered == 0)
    	{
    		/*
    		 * If no separator found, this means that the whole original-string is the token.
    		 */
    		token_end_pos = strlen(original);
    	}
    	else
    	{
    		*res = NULL;
    		return;
    	}
    }

    buffer_index = 0;
    for(tmp_index = token_start_pos; tmp_index < token_end_pos; tmp_index++)
    {
    	temp[buffer_index] = original[tmp_index];
    	buffer_index++;
    }

    temp[tmp_index] = 0;
    *res = temp;
}


void strip_leading_and_trailing_white_paces(char *buffer)
{
	int i = 0;

	/*
	 * Remove the trailing \r and \n (if any).
	 */
	for(i = strlen(buffer) - 1; i >= 0; i--)
	{
		if( (buffer[i] == '\r') || (buffer[i] == '\n') )
		{
			buffer[i] = 0;
		}
		else
		{
			break;
		}
	}

	while( (buffer[0] == '\r') || (buffer[0] == '\n') )
	{
		int length = strlen(buffer);
		for(i = 0; i < length; i++)
		{
			buffer[i] = buffer[i + 1];
		}

		buffer[i] = 0;
	}
}


#if 0
int main()
{
    printf("%d\n", sg_atoi("   123    "));
    printf("%d\n", sg_atoi(" ajay123  "));
    printf("%d\n", sg_atoi("  123ajay  "));
    printf("%d\n", sg_atoi("  ajay123ajay  "));
    printf("%d\n", sg_atoi("  -123  "));
    printf("%d\n", sg_atoi(" -ajay123  "));
    printf("%d\n", sg_atoi("  -123ajay  "));
    printf("%d\n", sg_atoi("  -ajay123ajay "));
}
#endif
