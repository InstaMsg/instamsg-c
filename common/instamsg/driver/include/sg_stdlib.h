#ifndef INSTAMSG_STDLIB
#define INSTAMSG_STDLIB

int sg_atoi(const char *buf);
char* sg_strnstr(const char *str1, const char *str2, int maxSize);
void get_nth_token(char *original, char separator, int pos, char **res);

#endif
