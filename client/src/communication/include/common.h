#define COMMUNICATION_FIELDS void *medium;                                                  \
                                                                                            \
                             int (*read)(Network* n, unsigned char* buffer, int len);       \
                             int (*write)(Network* n, unsigned char* buffer, int len);

