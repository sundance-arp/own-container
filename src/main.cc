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
#include <sys/ptrace.h>
#include <sys/user.h>
#include <time.h>
#include <limits.h>
#include <getopt.h>
#include <map>
#include <string>


#include "mount.h"
#include "namespace.h"
#include "utils.h"
#include "cgroups.h"
#include "ptrace.h"

#define CONTAINER_NAME_MAX 200
#define STACK_SIZE (1024 * 1024)


static char child_stack[STACK_SIZE];


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


std::map<std::string,const char *> parse_argument(int argc, char* argv[])
{
  int opt;
  // optionの結果が入るmap
  std::map<std::string,const char *> command_options;


  // 引数がなくなるまで回す
  while((opt = getopt(argc, argv, "fgh:tP")) != -1) {
    switch(opt) {
      case 't':
        {
          command_options.insert(std::make_pair("trace", ""));
          break;
        }

      case 'P':
        {
          command_options.insert(std::make_pair("privilege", ""));
          if(getuid() != 0){
            printf("privilege container is need root\n");
            exit(-1);
          }
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
          printf("* [-P] execute privilege container\n");
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


int create_container(void * arg){
  struct container_arg *ca = (container_arg *)arg;
  char ch;
  close(ca->pipe_fd[1]);
  if (read(ca->pipe_fd[0], &ch, 1) != 0) {
    printf("pipe read from child Error \n");
    exit(EXIT_FAILURE);
  }

  if(ca->command_options.count("trace") > 0){
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);
  }
  int rc = mount_cgroup_fs();
  if(rc < 0){
    return rc;
  }

  rc = chdir(ca->command_options.at("rootfs_path"));
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

  sethostname(ca->container_name,strlen(ca->container_name));

  rc = execl("/bin/sh","/bin/sh", NULL);
  if(rc < 0){
    printf("exec Error: %d\n", rc);
    return(-1);
  }
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


  // コンテナプロセス実行
  struct container_arg ca;
  rc = set_container_arg(&ca,command_options, container_name);
  if (rc < 0){
    return(rc);
  }
  int flags = get_clone_flags(command_options);
  int child_pid = clone(create_container, child_stack + STACK_SIZE,flags | SIGCHLD, &ca);

  // 親プロセス
  printf("Container PID: %d\n",child_pid);

  // cgroup関連の設定
  char container_cgroups_pid_dir[PATH_MAX];
  snprintf(container_cgroups_pid_dir, PATH_MAX, "/sys/fs/cgroup/pids/%s", container_name);
  create_container_cgroups_directory(container_cgroups_pid_dir);
  write_tasks_container_pid(container_cgroups_pid_dir, child_pid);
  write_pid_max(container_cgroups_pid_dir, 100);

  // ユーザーとグループのマッピング
  // 非特権コンテナ時(デフォルト)
  if(ca.command_options.count("privilege") <= 0){
    char map_file_path[PATH_MAX];
    setgroups_control(child_pid);
    snprintf(map_file_path, PATH_MAX, "/proc/%d/uid_map", child_pid);
    map_id(map_file_path, 0, getuid());
    snprintf(map_file_path, PATH_MAX, "/proc/%d/gid_map", child_pid);
    map_id(map_file_path, 0, getgid());
  }

  // コンテナプロセスを再開させる
  close(ca.pipe_fd[1]);

  wait_container_process(child_pid,container_name, command_options);
  return 0;
}

