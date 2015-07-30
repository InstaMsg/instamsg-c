/*******************************************************************************
 * Contributors:
 *
 *      Ajay Garg <ajay.garg@sensegrow.com>
 *
 *******************************************************************************/

#ifndef INSTAMSG_SERIAL
#define INSTAMSG_SERIAL

typedef struct Serial Serial;
struct Serial
{
    void *medium;

    int (*read)(Serial *serial, unsigned char* buffer, int len);
    int (*write)(Serial *serial, unsigned char* buffer, int len);
};

/*
 * In all likelihood, "arg" here will be a struct itself,
 * as any serial interface (eg. a serial-port) will have quite a bit
 * of parameters needed for it to be set up.
 */
Serial* get_new_serial_interface(void *arg);
void release_serial_interface(Serial*);

#endif
