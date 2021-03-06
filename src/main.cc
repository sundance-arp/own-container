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


#ifndef NAMESPACE_H
#define NAMESPACE_H
#include "namespace.h"
#endif

#ifndef COMMANDARGUMENT_H
#define COMMANDARGUMENT_H
#include "commandargument.h"
#endif

#include "mount.h"
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
      break;
    }
    ptrace(PTRACE_GETREGS, pid, NULL, &regs);
    if (regs.rax == - (long long unsigned int)ENOSYS) {
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

int wait_container_process(int pid,char *container_name, command_argument command_arg){
  int status;
  if(command_arg.trace){
    status = trace_container_systemcall(pid, container_name);
  }else {
    waitpid(pid,&status,WUNTRACED);
  }
  char rmdir_path[PATH_MAX];
  snprintf(rmdir_path, PATH_MAX, "/sys/fs/cgroup/pids/%s", container_name);
  rmdir(rmdir_path);
  return status;
}


command_argument parse_argument(int argc, char* argv[])
{
  int opt;
  // optionの結果が入るmap
  command_argument command_arg = init_command_argument();


  // 引数がなくなるまで回す
  while((opt = getopt(argc, argv, "fh:tPb:")) != -1) {
    switch(opt) {
      case 't':
        {
          command_arg.trace = true;
          break;
        }

      case 'P':
        {
          if(getuid() != 0){
            printf("privilege container is need root\n");
            exit(-1);
          }
          command_arg.privilege = true;
          break;
        }
      case 'b':
        {
          command_arg.bind = true;
          command_arg.bind_paths.push_back(optarg);
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
          printf("* [-b <host-absolute-path>:<container-absolute-path>] mount host path into container\n");
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
    command_arg.rootfs_path = argv[optind];
  }

  return command_arg;
}


int create_container(void * arg){
  struct container_arg *ca = (container_arg *)arg;
  char ch;
  close(ca->pipe_fd[1]);
  if (read(ca->pipe_fd[0], &ch, 1) != 0) {
    printf("pipe read from child Error \n");
    exit(EXIT_FAILURE);
  }

  if(ca->command_arg.trace){
    ptrace(PTRACE_TRACEME, 0, NULL, NULL);
  }
  int rc = mount_cgroup_fs();
  if(rc < 0){
    return rc;
  }

  if(ca->command_arg.bind){
    rc = mount_host_path(ca->command_arg.bind_paths, ca->command_arg.rootfs_path);
    if(rc < 0){
      return(rc);
    }
  }

  rc = chdir(ca->command_arg.rootfs_path.c_str());
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

  char *execarg[] = {(char *)"sh", NULL};
  char *execenv[] = {NULL};

  rc = execve("/bin/sh", execarg, execenv);
  if(rc < 0){
    printf("exec Error: %d\n", rc);
    return(-1);
  }
  return 0;
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
  command_argument command_arg = parse_argument(argc,argv);

  char container_name[CONTAINER_NAME_MAX];
  set_container_name(command_arg.rootfs_path.c_str(), container_name);
  printf("Container Name: %s\n",container_name);

  int rc=0;

  rc = mount_host_root();
  if(rc < 0){
    printf("/ mount Error: %d\n", rc);
    return(rc);
  }


  // コンテナプロセス実行
  struct container_arg ca;
  rc = set_container_arg(&ca,command_arg, container_name);
  if (rc < 0){
    return(rc);
  }
  int flags = get_clone_flags(command_arg);
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
  if(!ca.command_arg.privilege){
    char map_file_path[PATH_MAX];
    setgroups_control(child_pid);
    snprintf(map_file_path, PATH_MAX, "/proc/%d/uid_map", child_pid);
    map_id(map_file_path, 0, getuid());
    snprintf(map_file_path, PATH_MAX, "/proc/%d/gid_map", child_pid);
    map_id(map_file_path, 0, getgid());
  }

  // コンテナプロセスを再開させる
  close(ca.pipe_fd[1]);

  wait_container_process(child_pid,container_name, command_arg);
  return rc;
}

