#ifdef FILE_SYSTEM_INTERFACE_ENABLED

#ifndef INSTAMSG_FILE_SYSTEM_COMMON
#define INSTAMSG_FILE_SYSTEM_COMMON

#include "device_file_system.h"

/*
 * Global-functions callable.
 */
void init_file_system(FileSystem *fs, void *arg);
void release_file_system(FileSystem *fs);


/*
 * Must not be called directly.
 * Instead must be called as ::
 *
 *      fs->read
 *      fs->write
 *      fs->renameFile
 *      fs->deleteFile
 *      fs->getFileListing
 *      fs->getFileSize
 */
int file_system_read (FileSystem *fs, unsigned char* buffer, int len, unsigned char guaranteed);
int file_system_write(FileSystem *fs, unsigned char* buffer, int len);
int renameFile(FileSystem *fs, const char *oldPath, const char *newPath);
int deleteFile(FileSystem *fs, const char *filePath);
void getFileListing(FileSystem *fs, char *buf, int maxValueLenAllowed, const char *directoryPath);
long getFileSize(FileSystem *fs, const char *filepath);


/*
 * Internally Used.
 * Must not be called by anyone.
 */
void connect_underlying_file_system_medium_guaranteed(FileSystem* fs);
void release_underlying_file_system_medium_guaranteed(FileSystem* fs);



#endif

#endif
