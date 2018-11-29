#define _GNU_SOURCE
#include <sys/mount.h>
#include <sched.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <limits.h>

#include "mount-host.h"
#include "unshare-namespace.h"
#include "utils.h"
#include "manage-cgroups.h"


int wait_container_process(int pid){
  int status;
  waitpid(pid,&status,WUNTRACED);
  rmdir("/sys/fs/cgroup/pids/alpine-test");
  return status;
}

int main(int argc, char *argv[])
{
  int rc=0;

  rc = mount_host_root();
  if(rc < 0){
    printf("/ mount Error: %d\n", rc);
    return(rc);
  }

  rc = unshare_namespace();
  if(rc < 0){
    printf("unshare Error: %d\n", rc);
  }

  int pid = fork();
  switch(pid){
    // error
    case -1:
      printf("fork Error: %d\n", pid);
      break;
      // child
    case 0:
      break;
      // parent
    default:
      {
        int status = wait_container_process(pid);
        if(WIFEXITED(status)){
          printf("exit status = %d\n",WEXITSTATUS(status));
          return WEXITSTATUS(status);
        }else{
          printf("exit abnomally\n");
          return -1;
        }
      }
      break;
  }

  mount_cgroup_fs();

  char container_cgroups_pid_dir[] = "/sys/fs/cgroup/pids/alpine-test";

  create_container_cgroups_directory(container_cgroups_pid_dir);

  write_tasks_container_pid(container_cgroups_pid_dir);
 
  write_pid_max(container_cgroups_pid_dir, 100);

  rc = chdir("./alpine-test");
  if(rc < 0){
    printf("chdir Error: %d\n", rc);
  }
  rc = chroot("./");
  if(rc < 0){
    printf("chroot Error: %d\n", rc);
    return(-1);
  }

  rc = mount("proc", "./proc", "proc", MS_NOSUID|MS_NOEXEC|MS_NODEV, NULL);
  if(rc < 0){
    printf("proc mount Error: %d\n", rc);
    return(-1);
  }
  rc = mount("dev","./dev", "dev", MS_BIND | MS_REC, 0);
  if(rc < 0){
    printf("dev mount Error: %d\n", rc);
    return(-1);
  }

  //rc = mount("devpts","/dev/pts", "devpts", MS_BIND, 0);
  //if(rc < 0){
  //    printf("devpts mount Error: %d\n", rc);
  //    return(-1);
  //}
  //mount -t devpts devpts /dev/pts
  //mount("devpts", "/dev/pts", "devpts",0,0);

  rc = execl("/bin/sh","/bin/sh", NULL);
  if(rc < 0){
    printf("exec Error: %d\n", rc);
    return(-1);
  }

}

