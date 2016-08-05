#ifndef INSTAMSG_COMMON_HEX_UTILS
#define INSTAMSG_COMMON_HEX_UTILS

int getIntValueOfHexChar(char c);
void getByteStreamFromHexString(const char *hexString, unsigned char *buffer);
void addPaddingIfRequired(char *buf, int size);

#endif
