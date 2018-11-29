#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>

int mount_host_root(){
  return mount("none", "/", NULL, MS_PRIVATE | MS_REC, NULL);
}
