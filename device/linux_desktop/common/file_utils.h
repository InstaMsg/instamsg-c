#if FILE_SYSTEM_ENABLED == 1

void sg_readLine(FILE_STRUCT *fp, char *buffer, int maxBufferLength);
int sg_appendLine(const char *filePath, const char *buffer);
int sg_createEmptyFile(const char *filePath);

#endif
