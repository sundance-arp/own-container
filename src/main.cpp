#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

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
#include <sys/ptrace.h>
#include <sys/user.h>
#include <time.h>
#include <getopt.h>
#include <map>
#include <string>


#include "mount.h"
#include "namespace.h"
#include "utils.h"
#include "cgroups.h"
#include "ptrace.h"

#define CONTAINER_NAME_MAX 200

int trace_container_systemcall(int pid,char *container_name){
  struct user_regs_struct regs;
  int status;
  FILE *ptrace_log;
  ptrace_log = fopen("./tracelog","a+");

  while(1){
    ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
    waitpid(pid,&status,WUNTRACED);
    if(WIFEXITED(status)){
      printf("exit status = %d\n",WEXITSTATUS(status));
      break;
    }
    ptrace(PTRACE_GETREGS, pid, NULL, &regs);
    if (regs.rax == - ENOSYS) {
      continue;
    }
    time_t now_time = time(NULL);
    char* now_time_string = ctime(&now_time);
    now_time_string[strlen(now_time_string)-1] = '\0';
    fprintf(ptrace_log,"[%s] %s\n",now_time_string,systemcall_table[regs.orig_rax]);
  }

  fclose(ptrace_log);
  return status;
}

int wait_container_process(int pid,char *container_name, std::map<std::string,const char *> command_options){
  int status;
  if(command_options.count("trace") == 0){
    waitpid(pid,&status,WUNTRACED);
  }else {
    trace_container_systemcall(pid, container_name);
  }
  char rmdir_path[PATH_MAX];
  snprintf(rmdir_path, PATH_MAX, "/sys/fs/cgroup/pids/%s", container_name);
  rmdir(rmdir_path);
  return status;
}


int check_argument(int argc,char *argv[]){
  if(argc < 2){
    printf("rootを指定してください");
    exit(0);
  }

  return 0;
}


// TODO: オプション機能の導入
std::map<std::string,const char *> parse_argument(int argc, char* argv[])
{
  int opt;
  // optionの結果が入るmap
  std::map<std::string,const char *> command_options;


  // 引数がなくなるまで回す
  while((opt = getopt(argc, argv, "fgh:t")) != -1) {
    switch(opt) {
      case 't':
        {
          command_options.insert(std::make_pair("trace", ""));
          break;
        }

        // getoptの例
        //case 'f':
        //  {
        //    printf("-fがオプションとして渡されました\n");
        //    break;
        //  }

        //case 'h':
        //  {
        //    printf("-hがオプションとして渡されました\n");
        //    printf("引数optarg = %s\n", optarg);
        //    break;
        //  }

      default:
        {
          printf("Usage:\n");
          printf("* [-t] trace systemcall\n");
          exit(1);
        }
    }
  }

  // オプションではない引数の数を数える
  int no_opt_argument = 0;
  for (int i = optind; i < argc; i++) {
    no_opt_argument++;
  }

  // オプション以外の引数はrootfsのパスのみ
  if(no_opt_argument < 1){
    printf("rootを指定してください\n");
    exit(1);
  }else if(no_opt_argument > 1){
    printf("無効なオプションです\n");
    exit(1);
  }else{
    command_options.insert(std::make_pair("rootfs_path", argv[optind]));
  }

  return command_options;
}


int set_container_name(const char *rootfs_path,char *container_name){
  char absolute_path[PATH_MAX];
  realpath(rootfs_path, absolute_path);
  char *splitted_path[SPLIT_MAX];
  char slash[]="/";
  int count = split(absolute_path,slash,splitted_path);
  strncpy(container_name,splitted_path[count],CONTAINER_NAME_MAX);
  return 0;
}


int main(int argc, char *argv[])
{
  //check_argument(argc,argv);
  std::map<std::string,const char *> command_options = parse_argument(argc,argv);

  char container_name[CONTAINER_NAME_MAX];
  set_container_name(command_options.at("rootfs_path"), container_name);
  printf("Container Name: %s\n",container_name);

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
      if(command_options.count("trace") > 0){
        ptrace(PTRACE_TRACEME, 0, NULL, NULL);
      }
      break;
      // parent
    default:
      {
        wait_container_process(pid,container_name, command_options);
        return 0;
      }
      break;
  }

  rc = mount_cgroup_fs();
  if(rc < 0){
    return rc;
  }


  char container_cgroups_pid_dir[PATH_MAX];
  snprintf(container_cgroups_pid_dir, PATH_MAX, "/sys/fs/cgroup/pids/%s", container_name);

  create_container_cgroups_directory(container_cgroups_pid_dir);

  write_tasks_container_pid(container_cgroups_pid_dir, pid);

  write_pid_max(container_cgroups_pid_dir, 100);

  rc = chdir(command_options.at("rootfs_path"));
  if(rc < 0){
    printf("chdir Error: %d\n", rc);
  }

  rc = mount_dev_fs();
  if(rc < 0){
    return(rc);
  }

  rc = mount_proc_fs();
  if(rc < 0){
    return(rc);
  }

  rc = chroot("./");
  if(rc < 0){
    printf("chroot Error: %d\n", rc);
    return(-1);
  }

  sethostname(container_name,strlen(container_name));

  rc = execl("/bin/sh","/bin/sh", NULL);
  if(rc < 0){
    printf("exec Error: %d\n", rc);
    return(-1);
  }

}

