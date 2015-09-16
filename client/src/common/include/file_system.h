#ifdef FILE_SYSTEM_INTERFACE_ENABLED

#ifndef INSTAMSG_FILE_SYSTEM_COMMON
#define INSTAMSG_FILE_SYSTEM_COMMON

void init_file_system(FileSystem *fs, void *arg);
void release_file_system(FileSystem *fs);

void connect_underlying_file_system_medium_guaranteed(FileSystem* fs);
int file_system_read (FileSystem *fs, unsigned char* buffer, int len, unsigned char guaranteed);
int file_system_write(FileSystem *fs, unsigned char* buffer, int len);
int renameFile(FileSystem *fs, const char *oldPath, const char *newPath);
int deleteFile(FileSystem *fs, const char *filePath);
void getFileListing(FileSystem *fs, char *buf, int maxValueLenAllowed, const char *directoryPath);
long getFileSize(FileSystem *fs, const char *filepath);
void release_underlying_file_system_medium_guaranteed(FileSystem* fs);

#endif

#endif
