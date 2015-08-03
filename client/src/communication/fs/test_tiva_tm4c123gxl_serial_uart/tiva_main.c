#include "../../include/fs.h"
#include <string.h>

int main()
{
    FileSystem *fs = get_new_file_system("");

    unsigned char *str = (unsigned char*)"REALLY LONG STRING TO BEGIN WITH ::";
    fs->write(fs, str, strlen((const char*)str));

    while(1)
    {
        unsigned char buf[2] = {0};
        fs->read(fs, buf, 1);
        fs->write(fs, buf, 1);
    }

    release_file_system(fs);
    return 0;
}
