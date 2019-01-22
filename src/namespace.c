#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <fcntl.h>
#include <limits.h>


int unshare_namespace(){
  int flags = 0;
  flags |= CLONE_NEWPID;
  flags |= CLONE_NEWNS;
  flags |= CLONE_NEWUTS;
  flags |= CLONE_NEWIPC;
  flags |= CLONE_NEWUSER;
  //flags |= CLONE_NEWNET;
  return unshare(flags);
}


void setgroups_control(int pid){
  char map_file_path[PATH_MAX] = {0};
  snprintf(map_file_path, PATH_MAX, "/proc/%d/setgroups", pid);
  const char *cmd = "deny";
  FILE *fd;


  fd = fopen(map_file_path,"w");
  if (fd == NULL) {
    printf("setgroups file open Error\n");
    exit(1);
  }

  fprintf(fd,"%s",cmd);
  fclose(fd);
}


void map_id(const char *file, int from, int to){
  char buf[120];
  FILE *fd;

  fd = fopen(file,"w");
  if (fd == NULL) {
    printf("uid_map or gid_map file open Error\n");
    exit(1);
  }

  snprintf(buf, 120, "%d %d 1", from, to);
  fprintf(fd,"%s",buf);
  fclose(fd);
}
