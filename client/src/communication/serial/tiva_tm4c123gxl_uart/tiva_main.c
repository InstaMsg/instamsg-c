#include "../../../common/include/instamsg_vendor_common.h"

#include "instamsg_vendor.h"

#include <string.h>

int main()
{
    Serial serial;
    init_serial_interface(&serial, NULL);

    unsigned char *str = (unsigned char*)"TREATING UART AS SERIAL NOW (and not as FS) :: ";
    serial.write(&serial, str, strlen((const char*)str));

    while(1)
    {
        unsigned char buf[2] = {0};
        serial.read(&serial, buf, 1);
        serial.write(&serial, buf, 1);
    }

    release_serial_interface(&serial);
    return 0;
}
