#if FILE_SYSTEM_ENABLED == 1

void readLine(FILE_STRUCT *fp, char *buffer, int maxBufferLength);
int appendLine(const char *filePath, char *buffer);

#endif
