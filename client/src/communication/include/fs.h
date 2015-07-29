#ifndef INSTAMSG_FS
#define INSTAMSG_FS

typedef struct FileSystem FileSystem;
struct FileSystem
{
    void *medium;

    int (*read)(FileSystem *fs, unsigned char* buffer, int len);
    int (*write)(FileSystem *fs, unsigned char* buffer, int len);
};

FileSystem* get_new_file_system(void *arg);
void release_file_system(FileSystem*);

#endif
