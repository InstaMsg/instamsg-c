#define COMMUNICATION_FIELDS void *medium;                                                  \
                                                                                            \
                             void (*read)(Network* n, unsigned char* buffer, int len);      \
                             void (*write)(Network* n, unsigned char* buffer, int len);

