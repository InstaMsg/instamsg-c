#include "../../include/fs.h"

int main()
{
    FileSystem *fs = get_new_file_system("");

    unsigned char *str = (unsigned char*)"BEGIN ::";
    fs->write(fs, str, 8);

    while(1)
    {
        unsigned char buf[2] = {0};
        fs->read(fs, buf, 1);
        fs->write(fs, buf, 1);
    }

    release_file_system(fs);
    return 0;
}
