#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <fcntl.h>


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


void setgroups_control(int action){
  const char *file = "/proc/self/setgroups";
  const char *cmd;
  FILE *fd;

  cmd = "deny";

  fd = fopen(file,"w");
  if (fd == NULL) {
    printf("setgroups file open Error\n");
    exit(1);
  }

  fprintf(fd,"%s",cmd);
  fclose(fd);
}
