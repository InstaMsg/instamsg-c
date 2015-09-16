#ifdef FILE_SYSTEM_INTERFACE_ENABLED

void init_file_system(FileSystem *fs, void *arg)
{
    /* Register read-callback. */
	fs->read = file_system_read;

    /* Register write-callback. */
	fs->write = file_system_write;

    /* Register other-callbacks. */
    fs->renameFile = renameFile;
    fs->deleteFile = deleteFile;
    fs->getFileListing = getFileListing;
    fs->getFileSize = getFileSize;

    fs->fileName = (char*)arg;
    connect_underlying_file_system_medium_guaranteed(fs);
}

void release_file_system(FileSystem *fs)
{
    release_underlying_file_system_medium_guaranteed(fs);
}

#endif
