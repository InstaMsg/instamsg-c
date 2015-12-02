#ifndef IOEYE_HEX
#define IOEYE_HEX

int getIntValueOfHexChar(char c);
void getByteStreamFromHexString(const char *hexString, unsigned char *buffer);
void addPaddingIfRequired(char *buf, int size);

#endif
