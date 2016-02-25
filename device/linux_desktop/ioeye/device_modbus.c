#include "../../../common/instamsg/driver/include/globals.h"

#include "device_modbus.h"

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


#define PORT_NAME       PROSTR("/dev/ttyUSB0")


/*
 * This method initializes and connects to the Modbus-interface.
 */
void connect_underlying_modbus_medium_guaranteed(Modbus *modbus)
{
    modbus->fd = open(PORT_NAME, O_RDWR | O_NOCTTY | O_SYNC);
    if(modbus->fd < 0)
    {
        sg_sprintf(LOG_GLOBAL_BUFFER, PROSTR("error %u opening %s: %s"), errno, PORT_NAME, strerror (errno));
        error_log(LOG_GLOBAL_BUFFER);

        goto error_while_init;
    }

    if(set_interface_attribs(modbus->fd, B9600, 0) != SUCCESS)  /* set speed to 9600 bps, 8n1 (no parity) */
    {
        goto error_while_init;
    }

    if(set_blocking(modbus->fd, 0) != SUCCESS) /* set no blocking */
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


/*
 * This method ::
 *
 * a)
 * Sends the command to the modbus-interface.
 *
 * b)
 * Receives "responseBytesLength" number of bytes from the interface.
 *
 * Note that the calling-function must provide the number of bytes (responseBytesLength), since
 * the number of expected bytes is always calculatable from the sending-command itself.
 *
 *
 * The function must return exactly one of the following ::
 *
 * *
 * SUCCESS, if everything went fine.
 *
 * *
 * FAILURE, if "responseBytesLength" could not be received.
 *
 *
 * In general, modbus-interface (unlike a network-interface) is not expected to take too long in returning the response.
 * It will either return the response quickly, or will never.
 *
 * So, it is advisable (but not necessary) to wrap this function in a watchdog by the device-impementors.
 * If the modbus-response (equal to "responseBytesLength") is not available soonish, the device should be reset.
 */
int modbus_send_command_and_read_response_sync(Modbus *modbus,
                                               unsigned char *commandBytes,
                                               int commandBytesLength,
                                               unsigned char *responseByteBuffer,
                                               int responseBytesLength)
{
    write(modbus->fd, commandBytes, commandBytesLength);

    {
        int bytes_received = 0;
        while(bytes_received < responseBytesLength)
        {
            bytes_received = bytes_received + read(modbus->fd, responseByteBuffer + bytes_received, responseBytesLength - bytes_received);
        }
    }

    return SUCCESS;
}


/*
 * This method cleans up the modbus-interface.
 */
void release_underlying_modbus_medium_guaranteed(Modbus *modbus)
{
}
