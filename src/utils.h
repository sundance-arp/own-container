#ifndef UTILS_H
#define UTILS_H

#include <stdlib.h>
#define SPLIT_MAX 200
int mkdir_p(const char *path,mode_t mode);
int split(char *input, char *separator, char* output[]);

#endif

