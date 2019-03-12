#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <string.h>

#include <vector>
#include <fstream>
#include <string>
#include <filesystem>

#include "utils.h"

using std::vector;
using std::string;

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

// split
std::vector<std::string> string_split(std::string base, std::string delimiter){ 
  std::vector<std::string> result;                                              
  int delimiter_index = 0;                                                      
  while(true){                                                                  
    delimiter_index = base.find(delimiter);                                     
    if(delimiter_index == (int)std::string::npos){                     
      // "hoge--fuga" : "f" is delimiter_index+2.                               
      result.push_back(base.substr(delimiter_index + delimiter.size()));        
      break;                                                                    
    }                                                                           
    // "hoge--fuga" : "hoge" is index 0 to delimiter_index position.            
    result.push_back(base.substr(0,delimiter_index));                           
    // "hoge--fuga" : "f" is delimiter_index+2.                               
    base = base.substr(delimiter_index + delimiter.size());                     
  }                                                                             
  return result;                                                                
}                                                                               

int mount_host_to_container(std::filesystem::path host_path, std::filesystem::path container_path, std::filesystem::path rootfs_path){
  // abspath check
  if(!host_path.is_absolute()){
    printf("host path is not absolute path Error: %d\n", -1);
    printf("host path is must absolute path.\n");
    return -1;
  }
  if(!container_path.is_absolute()){
    printf("container path is not absolute path Error: %d\n", -1);
    printf("container path is must absolute path.\n");
    return -1;
  }

  if (!std::filesystem::exists(rootfs_path)){
    printf("rootfs path invalid Error: %d\n",-1);
    printf("rootfs is not exist.\n");
    return -1;
  }

  if(!std::filesystem::is_directory(rootfs_path)){
    printf("rootfs path invalid Error%d\n",-1);
    printf("rootfs is not directory.\n");
    return -1;
  }

  // rootfs + container_path
  std::filesystem::path rootfs_and_container_path;
  if(rootfs_path.string().back() == '/'){
    rootfs_and_container_path = std::filesystem::path(rootfs_path.string().substr(0,rootfs_path.string().size()-1) + container_path.string());
  }else {
    rootfs_and_container_path = std::filesystem::path(rootfs_path.string() + container_path.string());
  }

  // exist check
  if(!std::filesystem::exists(host_path)){
    printf("host path not found Error: %d\n",-1);
    return -1;
  }

  // file type check
  if(std::filesystem::exists(rootfs_and_container_path)){
    // if container path is exists, then check filetype
    if(std::filesystem::is_directory(host_path) ^ std::filesystem::is_directory(rootfs_and_container_path)){
      std::string host_path_file_type;
      std::string rootfs_and_container_path_file_type;

      if(std::filesystem::is_directory(host_path)){
        host_path_file_type = "directory";
      }else {
        host_path_file_type = "not directory";
      }

      if(std::filesystem::is_directory(rootfs_and_container_path)){
        rootfs_and_container_path_file_type = "directory";
      }else {
        rootfs_and_container_path_file_type = "not directory";
      }

      printf("invalid mount point Error%d\n",-1);
      printf("host path is %s, but container path is %s.\n",host_path_file_type.c_str(), rootfs_and_container_path_file_type.c_str());
      return -1;
    }
  }else {
    // Create a mount point according to file type
    if(std::filesystem::is_directory(host_path)){
      mkdir_p(rootfs_and_container_path.c_str(), 0744);
    }else {
      std::ofstream(rootfs_and_container_path.string());
      std::filesystem::permissions(rootfs_and_container_path, std::filesystem::perms::owner_all | std::filesystem::perms::group_read | std::filesystem::perms::others_read);
    }
  }

  int rc = mount(host_path.string().c_str(), (rootfs_path.string() + "/" + container_path.string()).c_str(), NULL, MS_BIND|MS_REC, NULL);
  if(rc < 0){
    printf("host path mount to container Error: %d\n", rc);
    return(-1);
  }
  return 0;
}

int mount_host_path(vector<string> mount_paths, std::filesystem::path rootfs_path){
  using std::vector;
  using std::string;
  for (string mount_path : mount_paths) {
    vector<string> from_to_paths = string_split(mount_path, ":");
    if(from_to_paths.size() != 2){
      printf("mount option format Error \n");
      return -1;
    }

    int rc = mount_host_to_container(std::filesystem::path(from_to_paths[0]),std::filesystem::path(from_to_paths[1]),std::filesystem::path(rootfs_path));
    if(rc < 0){
      return rc;
    }
  }
  return 0;
}
