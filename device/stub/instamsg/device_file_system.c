#include "./device_file_system.h"
#include "../../../common/instamsg/driver/include/globals.h"


#if FILE_SYSTEM_ENABLED == 1

/*
 * This method MUST connect the underlying medium (even if it means to retry continuously).
 */
void connect_underlying_file_system_medium_guaranteed(FileSystem* fs)
{
}


/*
 * This method reads first "len" bytes from file-stream into "buffer".
 *
 * This is a blocking function. So, either of the following must hold true ::
 *
 * a)
 * "len" bytes are read.
 * In this case, SUCCESS must be returned.
 *
 *                      OR
 * b)
 * An error occurred while reading.
 * In this case, FAILURE must be returned immediately.
 */
int file_system_read(FileSystem *fs, unsigned char* buffer, int len, unsigned char guaranteed)
{
    return FAILURE;
}


/*
 * This method writes first "len" bytes from "buffer" onto the network.
 *
 * This is a blocking function. So, either of the following must hold true ::
 *
 * a)
 * All "len" bytes are written.
 * In this case, SUCCESS must be returned.
 *
 *                      OR
 * b)
 * An error occurred while writing.
 * In this case, FAILURE must be returned immediately.).
 */
int file_system_write(FileSystem *fs, unsigned char* buffer, int len)
{
    return FAILURE;
}


/*
 * This method renames/moves a file on the file-system.
 */
int renameFile(FileSystem *fs, const char *oldPath, const char *newPath)
{
    return FAILURE;
}



/*
 * This method copies a file on the file-system.
 */
int copyFile(FileSystem *fs, const char *oldPath, const char *newPath)
{
    return FAILURE;
}


/*
 * This method deletes a file from the file-system.
 */
int deleteFile(FileSystem *fs, const char *filePath)
{
    return FAILURE;
}


/*
 * This method returns the file-listing in the directory specified, and populates the "buf".
 * The format is ::
 *
 *          {"publisher.sh":152,"subscriber.sh":140,"filetester.sh":137,"config.txt_filetester":148,"stdoutsub":56040,"build_ubuntu_14_04.sh":2892,"README.md":1502,"config.txt":128,"stdoutsub.c":6649,"config.txt_local_testing":128}
 */
void getFileListing(FileSystem *fs, char *buf, int maxValueLenAllowed, const char *directoryPath)
{
}


/*
 * This method returns a long-value, specifying the size of file in bytes.
 */
long getFileSize(FileSystem *fs, const char *filepath)
{
    return 0;
}


/*
 * This method MUST release the underlying medium (even if it means to retry continuously).
 * But if it is ok to re-connect without releasing the underlying-system-resource, then this can be left empty.
 */
void release_underlying_file_system_medium_guaranteed(FileSystem* fs)
{
}

#endif

