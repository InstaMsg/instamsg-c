#include "../../../common/include/instamsg_vendor_common.h"

#include "instamsg_vendor.h"

#include <string.h>

int main()
{
    FileSystem fs;
    init_file_system(&fs, "");

    unsigned char *str = (unsigned char*)"REALLY LONG STRING TO BEGIN WITH ::";
    fs.write(&fs, str, strlen((const char*)str));

    while(1)
    {
        unsigned char buf[2] = {0};
        fs.read(&fs, buf, 1);
        fs.write(&fs, buf, 1);
    }

    release_file_system(&fs);
    return 0;
}
