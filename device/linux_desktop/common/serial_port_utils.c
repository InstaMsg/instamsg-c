#include "../../../common/instamsg/driver/include/globals.h"

#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <string.h>


static int set_interface_attribs (int fd,
                                  int speed,
                                  int parity,
                                  int chars,
                                  int blocking)
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

        tty.c_cflag &= ~(PARENB | PARODD);
        tty.c_cflag |= parity;

        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

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


void connect_serial_port(int *fd, const char *port_name, int speed, int parity, int chars, int blocking)
{
    *fd = -1;

    *fd = open(port_name, O_RDWR | O_NOCTTY | O_SYNC);
    if(*fd < 0)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("error %u opening %s: %s"), errno, port_name, strerror (errno));
        error_log(LOG_GLOBAL_BUFFER);

        goto error_while_init;
    }

    if(set_interface_attribs(*fd, speed, parity, chars, blocking) != SUCCESS)
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
