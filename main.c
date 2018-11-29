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


int wait_container_process(int pid){
  int status;
  waitpid(pid,&status,WUNTRACED);
  rmdir("/sys/fs/cgroup/pids/alpine-test");
  return status;
}
int mkdir_p(const char *path,mode_t mode)
{
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

  // cgdirの存在確認
  struct stat st;
  // cgroupのディレクトリがあるかどうか
  stat("/sys/fs/cgroup", &st);
  if((st.st_mode  & S_IFMT) == S_IFDIR){
    // あればなにもしない
  }else {
    // なければ作る
    rc = mkdir_p("/sys/fs/cgroup",0755);
    if(rc < 0){
      printf("cgroup dir create Error: %d\n", rc);
      return(rc);
    }
    // なければマウント
    rc = mount("cgroup","/sys/fs/cgroup", "cgroup", 0, 0);
    if(rc < 0){
      printf("cgroup mount Error: %d\n", rc);
      return(rc);
    }
  }

  // コンテナ用のcgroupディレクトリ作成
  mkdir("/sys/fs/cgroup/pids/alpine-test",0755);
  // cgroupへpidの書き込み
  FILE *cgroup_file;
  cgroup_file = fopen("/sys/fs/cgroup/pids/alpine-test/tasks","a+");
  if (cgroup_file == NULL) {
    printf("cgroup task file open Error\n");
    exit(1);
  }
  // cgroupへpidの上限の書き込み
  int container_process_pid = getpid();
  fprintf(cgroup_file,"%d\n",container_process_pid);
  fclose(cgroup_file);
  cgroup_file = fopen("/sys/fs/cgroup/pids/alpine-test/pids.max","a+");
  if (cgroup_file == NULL) {
    printf("cannot open\n");
    exit(1);
  }
  fprintf(cgroup_file,"%d\n",100);
  fclose(cgroup_file);


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

