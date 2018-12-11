#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>

int unshare_namespace(){
  int flags = 0;
  flags |= CLONE_NEWPID;
  flags |= CLONE_NEWNS;
  flags |= CLONE_NEWUTS;
  flags |= CLONE_NEWIPC;
  //flags |= CLONE_NEWNET;
  return unshare(flags);
}
