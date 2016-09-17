/*******************************************************************************
 *
 * Copyright (c) 2014 SenseGrow, Inc.
 *
 * SenseGrow Internet of Things (IoT) Client Frameworks
 * http://www.sensegrow.com
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the
 * Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.

 * Contributors:
 *    Ajay Garg <ajay.garg@sensegrow.com>
 *******************************************************************************/



#include "../../../common/instamsg/driver/include/globals.h"
#include "../../../common/instamsg/driver/include/misc.h"
#include "../../../common/instamsg/driver/include/log.h"
#include "../../../common/instamsg/driver/include/sg_stdlib.h"


#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>

#include "./serial_port_utils.h"

struct baud_struct
{
    char *user_baud_string;
    char *baud_string;
    int baud;
};

struct baud_struct BAUD_RATES[] =
{
    {
        "1800",
        "B1800",
        B1800
    },
    {
        "2400",
        "B2400",
        B2400
    },
    {
        "4800",
        "B4800",
        B4800
    },
    {
        "9600",
        "B9600",
        B9600
    },
    {
        "19200",
        "B19200",
        B19200
    },
    {
        "38400",
        "B38400",
        B38400
    },
    {
        "57600",
        "B57600",
        B57600
    },
    {
        "115200",
        "B115200",
        B115200
    },
    {
        "230400",
        "B230400",
        B230400
    },
    {
        NULL,
        NULL,
        0
    }
};


struct char_struct
{
    char *user_chars_string;
    char *chars_string;
    int chars;
};

struct char_struct CHAR_COUNTS[] =
{
    {
        "5",
        "CS5",
        CS5
    },
    {
        "6",
        "CS6",
        CS6
    },
    {
        "7",
        "CS7",
        CS7
    },
    {
        "8",
        "CS8",
        CS8
    }
};



void parse_serial_connection_params(char *params_string,
                                    int *speed,
                                    int *parity,
                                    int *odd_parity,
                                    int *chars,
                                    int *blocking,
                                    int *two_stop_bits,
                                    int *hardware_control)

{
    char small[10];
    int i = 0;

    char *speed_string  = "";
    char *chars_string  = "";

    /*
     * First, set the default values, equal to 230400 7N1 No-Flow-Control.
     *                          B230400, 0, 0, CS7, 1, 0, 0
     */
    *speed              =   B230400;
    speed_string        =   "B230400";

    *parity             =   0;
    *odd_parity         =   0;

    *chars              =   CS7;
    chars_string        =   "CS7";

    *blocking           =   1;
    *two_stop_bits      =   0;
    *hardware_control   =   0;


    {
        int comma_count = get_character_count(params_string, ',');
        if(comma_count != 6)
        {
            sg_sprintf(LOG_GLOBAL_BUFFER, "Number of commas in serial-params [%s] is wrong, using default values", params_string);
            error_log(LOG_GLOBAL_BUFFER);

            goto exit;
        }
    }


    memset(small, 0, sizeof(small));
    get_nth_token_thread_safe(params_string, ',', 1, small, 1);
    i = 0;
    while(1)
    {
        if(BAUD_RATES[i].user_baud_string == NULL)
        {
            break;
        }

        if(strcmp(BAUD_RATES[i].user_baud_string, small) == 0)
        {
            *speed = BAUD_RATES[i].baud;
            speed_string = BAUD_RATES[i].baud_string;

            break;
        }
    }

    memset(small, 0, sizeof(small));
    get_nth_token_thread_safe(params_string, ',', 2, small, 1);
    *parity = sg_atoi(small);

    memset(small, 0, sizeof(small));
    get_nth_token_thread_safe(params_string, ',', 3, small, 1);
    *odd_parity = sg_atoi(small);

    memset(small, 0, sizeof(small));
    get_nth_token_thread_safe(params_string, ',', 4, small, 1);
    i = 0;
    while(1)
    {
        if(CHAR_COUNTS[i].user_chars_string == NULL)
        {
            break;
        }

        if(strcmp(CHAR_COUNTS[i].user_chars_string, small) == 0)
        {
            *chars = CHAR_COUNTS[i].chars;
            chars_string = CHAR_COUNTS[i].chars_string;

            break;
        }
    }

    memset(small, 0, sizeof(small));
    get_nth_token_thread_safe(params_string, ',', 5, small, 1);
    *blocking = sg_atoi(small);

    memset(small, 0, sizeof(small));
    get_nth_token_thread_safe(params_string, ',', 6, small, 1);
    *two_stop_bits = sg_atoi(small);

    memset(small, 0, sizeof(small));
    get_nth_token_thread_safe(params_string, ',', 7, small, 1);
    *hardware_control = sg_atoi(small);



exit:
    sg_sprintf(LOG_GLOBAL_BUFFER, "Extracted Serial-Params from [%s] :: \n\n"
                                  "Speed        =   [%s]\n"
                                  "Parity       =   [%u]\n"
                                  "Odd-Parity   =   [%u]\n"
                                  "Chars        =   [%s]\n"
                                  "Blocking     =   [%u]\n"
                                  "2-stop-bits  =   [%u]\n"
                                  "HW-control   =   [%u]\n",
               params_string, speed_string, *parity, *odd_parity, chars_string, *blocking, *two_stop_bits, *hardware_control);

    info_log(LOG_GLOBAL_BUFFER);
}



static int set_interface_attribs (int fd,
                                  int speed,
                                  int parity,
                                  int odd_parity,
                                  int chars,
                                  int blocking,
                                  int two_stop_bits,
                                  int hardware_control)
{
    {
        struct termios tty;
        memset (&tty, 0, sizeof(tty));

        if(tcgetattr(fd, &tty) != 0)
        {
            int errno_bkp = errno;

            sg_sprintf(LOG_GLOBAL_BUFFER, "error %u from tcgetattr", errno_bkp);
            error_log(LOG_GLOBAL_BUFFER);

            return FAILURE;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | chars;
        tty.c_iflag &= ~IGNBRK;

        tty.c_lflag = 0;
        tty.c_oflag = 0;

        if(blocking == 1)
        {
            tty.c_cc[VMIN]  = 1;
            tty.c_cc[VTIME] = 0;
        }
        else
        {
            tty.c_cc[VMIN]  = 0;
            tty.c_cc[VTIME] = 5;
        }

        tty.c_iflag &= ~(IXON | IXOFF | IXANY);
        tty.c_cflag |= (CLOCAL | CREAD);

        if(parity == 1)
        {
            if(odd_parity == 1)
            {
                tty.c_cflag |= (PARENB | PARODD);
            }
            else
            {
                tty.c_cflag |= (PARENB & (~PARODD));
            }
        }
        else
        {
            tty.c_cflag &= ~(PARENB | PARODD);
        }

        if(two_stop_bits == 1)
        {
            tty.c_cflag |= CSTOPB;
        }
        else
        {
            tty.c_cflag &= ~CSTOPB;
        }

        if(hardware_control == 1)
        {
            tty.c_cflag |= CRTSCTS;
        }
        else
        {
            tty.c_cflag &= ~CRTSCTS;
        }

        if(tcsetattr(fd, TCSANOW, &tty) != 0)
        {
            int errno_bkp = errno;

            sg_sprintf(LOG_GLOBAL_BUFFER, "error %u from tcsetattr", errno_bkp);
            error_log(LOG_GLOBAL_BUFFER);

            return FAILURE;
        }

        return SUCCESS;
    }
}


void connect_serial_port(int *fd, const char *port_name, char *params_string)
{
    int speed = 0, parity = 0, odd_parity = 0, chars = 0, blocking = 0, two_stop_bits = 0, hardware_control = 0;
    parse_serial_connection_params(params_string, &speed, &parity, &odd_parity, &chars, &blocking, &two_stop_bits, &hardware_control);

    *fd = -1;

    *fd = open(port_name, O_RDWR | O_NOCTTY | O_SYNC | O_NDELAY);
    if(*fd < 0)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("error %u opening %s: %s"), errno, port_name, strerror (errno));
        error_log(LOG_GLOBAL_BUFFER);

        goto error_while_init;
    }

    if(set_interface_attribs(*fd, speed, parity, odd_parity, chars, blocking, two_stop_bits, hardware_control) != SUCCESS)
    {
        goto error_while_init;
    }

    /*
     * If we reach till here, we instantiated everything fine.
     */
    return;

error_while_init:
    exitApp();
}


int disconnect_serial_port(int fd)
{
    if(fd < 0)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, "File-Descritor is less than zero .. exiting-process ....");
        error_log(LOG_GLOBAL_BUFFER);

        resetDevice();
        return FAILURE;
    }
    else
    {
        close(fd);
        return SUCCESS;
    }
}
