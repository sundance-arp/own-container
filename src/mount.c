#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/stat.h>

#include "utils.h"

int mount_host_root(){
  return mount("none", "/", NULL, MS_PRIVATE | MS_REC, NULL);
}

int mount_cgroup_fs(){
  // cgdirの存在確認
  struct stat st;
  // cgroupのディレクトリがあるかどうか
  stat("/sys/fs/cgroup", &st);
  if((st.st_mode  & S_IFMT) == S_IFDIR){
    // あればなにもしない
  }else {
    // なければ作る
    int rc = mkdir_p("/sys/fs/cgroup",0755);
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
  return 0;
}

int mount_dev_fs(){
  // ここのオプションは検討の余地あり
  int rc = mount("/dev", "./dev", NULL, MS_BIND|MS_REC|MS_NOSUID|MS_NODEV, NULL);
  if(rc < 0){
    printf("dev mount Error: %d\n", rc);
    return(-1);
  }
  return 0;
}

int mount_proc_fs(){
  int rc = mount("proc", "./proc", "proc", MS_NOSUID|MS_NOEXEC|MS_NODEV, NULL);
  if(rc < 0){
    printf("proc mount Error: %d\n", rc);
    return(-1);
  }
  return 0;
}

// TODO: Implement a function for options like -v
int mount_host_path(){
  int rc = mount("/home/sundance-arp", "./mnt/hdd", NULL, MS_BIND|MS_REC|MS_NOSUID|MS_NODEV|MS_RDONLY, NULL);
  if(rc < 0){
    printf("host path mount Error: %d\n", rc);
    return(-1);
  }
  return 0;
}
