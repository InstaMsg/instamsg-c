#include <string.h>
#include "../../../common/include/globals.h"

#include "instamsg_vendor.h"

int main()
{
    //unsigned char res[100] = {0};
    SerialLoggerInterface serial;
    init_serial_logger_interface(&serial, NULL);

    unsigned char buf[] = "Wow.. this works ";
    unsigned int len = sg_strlen((char*)buf);
    //sg_sprintf(res, "the length is %s", "123");
    //serial.write(&serial, res, 1);

    serial.write(&serial, buf, len);
    //serial.write(&serial, buf, 7);

    while(1)
    {
    }

    return 0;
}
