#include "../../../common/include/globals.h"

#include "instamsg_vendor.h"
#include <string.h>

int main()
{
    SerialLoggerInterface serial;
    unsigned char buf[] = "Wow.. this works ";
    init_serial_logger_interface(&serial, NULL);

    char res[100] = {0};
    memset(res, 0, 100);

    serial.write(&serial, buf, strlen((char *)buf));

    while(1)
    {
    }

    return 0;
}
