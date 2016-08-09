#include "../../../common/instamsg/driver/include/globals.h"

#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <string.h>


static int set_interface_attribs (int fd, int speed, int parity)
{
    {
        struct termios tty;

        memset (&tty, 0, sizeof(tty));
        if (tcgetattr(fd, &tty) != 0)
        {
            int errno_bkp = errno;

            sg_sprintf(LOG_GLOBAL_BUFFER, "error %u from tcgetattr", errno_bkp);
            error_log(LOG_GLOBAL_BUFFER);

            return FAILURE;
        }

        cfsetospeed (&tty, speed);
        cfsetispeed (&tty, speed);

        tty.c_cflag = (tty.c_cflag & ~CSIZE) | CS8;     /* 8-bit chars */

        /* disable IGNBRK for mismatched speed tests; otherwise receive break
         * as \000 chars
         */
        tty.c_iflag &= ~IGNBRK;                         /* disable break processing */
        tty.c_lflag = 0;                                /* no signaling chars, no echo, no canonical processing */
        tty.c_oflag = 0;                                /* no remapping, no delays */
        tty.c_cc[VMIN]  = 0;                            /* read doesn't block */
        tty.c_cc[VTIME] = 5;                            /* 0.5 seconds read timeout */
        tty.c_iflag &= ~(IXON | IXOFF | IXANY);         /* shut off xon/xoff ctrl */

        tty.c_cflag |= (CLOCAL | CREAD);                /* ignore modem controls, enable reading */
        tty.c_cflag &= ~(PARENB | PARODD);              /* shut off parity */
        tty.c_cflag |= parity;
        tty.c_cflag &= ~CSTOPB;
        tty.c_cflag &= ~CRTSCTS;

        if (tcsetattr(fd, TCSANOW, &tty) != 0)
        {
            int errno_bkp = errno;

            sg_sprintf(LOG_GLOBAL_BUFFER, "error %u from tcsetattr", errno_bkp);
            error_log(LOG_GLOBAL_BUFFER);

            return FAILURE;
        }

        return SUCCESS;
    }
}


static int set_blocking(int fd, int should_block)
{
    {
        struct termios tty;

        memset (&tty, 0, sizeof tty);
        if (tcgetattr(fd, &tty) != 0)
        {
            int errno_bkp = errno;

            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("error %u from tggetattr"), errno_bkp);
            error_log(LOG_GLOBAL_BUFFER);

            return FAILURE;
        }

        tty.c_cc[VMIN]  = should_block ? 1 : 0;
        tty.c_cc[VTIME] = 5;            /* 0.5 seconds read timeout */

        if (tcsetattr (fd, TCSANOW, &tty) != 0)
        {
            int errno_bkp = errno;

            sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("error %u setting term attributes"), errno_bkp);
            error_log(LOG_GLOBAL_BUFFER);

            return FAILURE;
        }
    }

    return SUCCESS;
}


void connect_serial_port(int *fd, const char *port_name, int speed, int parity)
{
    *fd = open(port_name, O_RDWR | O_NOCTTY | O_SYNC);
    if(*fd < 0)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("error %u opening %s: %s"), errno, port_name, strerror (errno));
        error_log(LOG_GLOBAL_BUFFER);

        goto error_while_init;
    }

    if(set_interface_attribs(*fd, speed, parity) != SUCCESS)
    {
        goto error_while_init;
    }

    if(set_blocking(*fd, 1) != SUCCESS) /* set no blocking */
    {
        goto error_while_init;
    }

    /*
     * If we reach till here, we instantiated everything fine.
     */
    return;

error_while_init:
    rebootDevice();
}
