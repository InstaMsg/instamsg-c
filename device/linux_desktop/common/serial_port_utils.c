#include "../../../common/instamsg/driver/include/globals.h"
#include "../../../common/instamsg/driver/include/misc.h"
#include "../../../common/instamsg/driver/include/log.h"

#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>


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
                tty.c_cflag &= (PARENB | PARODD);
            }
            else
            {
                tty.c_cflag &= (PARENB | (~PARODD));
            }
        }
        else
        {
            tty.c_cflag &= ~(PARENB | PARODD);
        }
        tty.c_cflag |= parity;

        if(two_stop_bits == 1)
        {
            tty.c_cflag &= CSTOPB;
        }
        else
        {
            tty.c_cflag &= ~CSTOPB;
        }

        if(hardware_control == 1)
        {
            tty.c_cflag &= CRTSCTS;
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


void connect_serial_port(int *fd,
                         const char *port_name,
                         int speed,
                         int parity,
                         int odd_parity,
                         int chars,
                         int blocking,
                         int two_stop_bits,
                         int hardware_control)
{
    *fd = -1;

    *fd = open(port_name, O_RDWR | O_NOCTTY | O_SYNC);
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
