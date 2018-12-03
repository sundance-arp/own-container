#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include "utils.h"

int mkdir_p(const char *path,mode_t mode){
  /* Adapted from http://stackoverflow.com/a/2336245/119527 */
  const size_t len = strlen(path);
  char _path[PATH_MAX];
  char *p;

  errno = 0;

  /* Copy string so its mutable */
  if (len > sizeof(_path)-1) {
    errno = ENAMETOOLONG;
    return -1;
  }
  strcpy(_path, path);

  /* Iterate the string */
  for (p = _path + 1; *p; p++) {
    if (*p == '/') {
      /* Temporarily truncate */
      *p = '\0';

      if (mkdir(_path, mode) != 0) {
        if (errno != EEXIST)
          return -1;
      }

      *p = '/';
    }
  }

  if (mkdir(_path, mode) != 0) {
    if (errno != EEXIST)
      return -1;
  }

  return 0;
}
int split(char *input, char *separator, char* output[]){
  char* tp;

  int count = 0;
  tp = strtok(input, separator);
  output[count] = tp;
  while(1){
    tp = strtok(NULL, separator);
    if(tp == NULL){
      break;
    }
    count++;
    output[count] = tp;
  }

  return count;
}
