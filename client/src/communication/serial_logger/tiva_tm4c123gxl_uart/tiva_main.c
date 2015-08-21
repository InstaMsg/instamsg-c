#include "../../../common/include/globals.h"

#include "instamsg_vendor.h"

#if 0
void test(char *buf)
{
}
#endif

int main()
{
    SerialLoggerInterface serial;
    unsigned char buf[] = "Wow.. this works ";
    init_serial_logger_interface(&serial, NULL);

    char res[100] = {0};
    memset(res, 0, 100);
    test(res);  // <== If this line is commened, the output is seen fine on the UART.

    serial.write(&serial, buf, strlen((char *)buf));

    while(1)
    {
    }

    return 0;
}
