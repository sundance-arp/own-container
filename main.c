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
#include <sys/ptrace.h>
#include<sys/user.h>

#include "mount.h"
#include "unshare-namespace.h"
#include "utils.h"
#include "cgroups.h"

#define CONTAINER_NAME_MAX 200
const char *systemcall_table[] = {
  "read",
  "write",
  "open",
  "close",
  "stat",
  "fstat",
  "lstat",
  "poll",
  "lseek",
  "mmap",
  "mprotect",
  "munmap",
  "brk",
  "rt_sigaction",
  "rt_sigprocmask",
  "rt_sigreturn",
  "ioctl",
  "pread",
  "pwrite",
  "readv",
  "writev",
  "access",
  "pipe",
  "select",
  "sched_yield",
  "mremap",
  "msync",
  "mincore",
  "madvise",
  "shmget",
  "shmat",
  "shmctl",
  "dup",
  "dup2",
  "pause",
  "nanosleep",
  "getitimer",
  "alarm",
  "setitimer",
  "getpid",
  "sendfile",
  "socket",
  "connect",
  "accept",
  "sendto",
  "recvfrom",
  "sendmsg",
  "recvmsg",
  "shutdown",
  "bind",
  "listen",
  "getsockname",
  "getpeername",
  "socketpair",
  "setsockopt",
  "getsockopt",
  "clone",
  "fork",
  "vfork",
  "execve",
  "exit",
  "wait4",
  "kill",
  "uname",
  "semget",
  "semop",
  "semctl",
  "shmdt",
  "msgget",
  "msgsnd",
  "msgrcv",
  "msgctl",
  "fcntl",
  "flock",
  "fsync",
  "fdatasync",
  "truncate",
  "ftruncate",
  "getdents",
  "getcwd",
  "chdir",
  "fchdir",
  "rename",
  "mkdir",
  "rmdir",
  "creat",
  "link",
  "unlink",
  "symlink",
  "readlink",
  "chmod",
  "fchmod",
  "chown",
  "fchown",
  "lchown",
  "umask",
  "gettimeofday",
  "getrlimit",
  "getrusage",
  "sysinfo",
  "times",
  "ptrace",
  "getuid",
  "syslog",
  "getgid",
  "setuid",
  "setgid",
  "geteuid",
  "getegid",
  "setpgid",
  "getppid",
  "getpgrp",
  "setsid",
  "setreuid",
  "setregid",
  "getgroups",
  "setgroups",
  "setresuid",
  "getresuid",
  "setresgid",
  "getresgid",
  "getpgid",
  "setfsuid",
  "setfsgid",
  "getsid",
  "capget",
  "capset",
  "rt_sigpending",
  "rt_sigtimedwait",
  "rt_sigqueueinfo",
  "rt_sigsuspend",
  "sigaltstack",
  "utime",
  "mknod",
  "uselib",
  "personality",
  "ustat",
  "statfs",
  "fstatfs",
  "sysfs",
  "getpriority",
  "setpriority",
  "sched_setparam",
  "sched_getparam",
  "sched_setscheduler",
  "sched_getscheduler",
  "sched_get_priority_max",
  "sched_get_priority_min",
  "sched_rr_get_interval",
  "mlock",
  "munlock",
  "mlockall",
  "munlockall",
  "vhangup",
  "modify_ldt",
  "pivot_root",
  "_sysctl",
  "prctl",
  "arch_prctl",
  "adjtimex",
  "setrlimit",
  "chroot",
  "sync",
  "acct",
  "settimeofday",
  "mount",
  "umount2",
  "swapon",
  "swapoff",
  "reboot",
  "sethostname",
  "setdomainname",
  "iopl",
  "ioperm",
  "create_module",
  "init_module",
  "delete_module",
  "get_kernel_syms",
  "query_module",
  "quotactl",
  "nfsservctl",
  "getpmsg",
  "putpmsg",
  "afs_syscall",
  "tuxcall",
  "security",
  "gettid",
  "readahead",
  "setxattr",
  "lsetxattr",
  "fsetxattr",
  "getxattr",
  "lgetxattr",
  "fgetxattr",
  "listxattr",
  "llistxattr",
  "flistxattr",
  "removexattr",
  "lremovexattr",
  "fremovexattr",
  "tkill",
  "time",
  "futex",
  "sched_setaffinity",
  "sched_getaffinity",
  "set_thread_area",
  "io_setup",
  "io_destroy",
  "io_getevents",
  "io_submit",
  "io_cancel",
  "get_thread_area",
  "lookup_dcookie",
  "epoll_create",
  "epoll_ctl_old",
  "epoll_wait_old",
  "remap_file_pages",
  "getdents64",
  "set_tid_address",
  "restart_syscall",
  "semtimedop",
  "fadvise64",
  "timer_create",
  "timer_settime",
  "timer_gettime",
  "timer_getoverrun",
  "timer_delete",
  "clock_settime",
  "clock_gettime",
  "clock_getres",
  "clock_nanosleep",
  "exit_group",
  "epoll_wait",
  "epoll_ctl",
  "tgkill",
  "utimes",
  "vserver",
  "mbind",
  "set_mempolicy",
  "get_mempolicy",
  "mq_open",
  "mq_unlink",
  "mq_timedsend",
  "mq_timedreceive",
  "mq_notify",
  "mq_getsetattr",
  "kexec_load",
  "waitid",
  "add_key",
  "request_key",
  "keyctl",
  "ioprio_set",
  "ioprio_get",
  "inotify_init",
  "inotify_add_watch",
  "inotify_rm_watch",
  "migrate_pages",
  "openat",
  "mkdirat",
  "mknodat",
  "fchownat",
  "futimesat",
  "newfstatat",
  "unlinkat",
  "renameat",
  "linkat",
  "symlinkat",
  "readlinkat",
  "fchmodat",
  "faccessat",
  "pselect6",
  "ppoll",
  "unshare",
  "set_robust_list",
  "get_robust_list",
  "splice",
  "tee",
  "sync_file_range",
  "vmsplice",
  "move_pages",
  "utimensat",
  "epoll_pwait",
  "signalfd",
  "timerfd",
  "eventfd",
  "fallocate",
  "timerfd_settime",
  "timerfd_gettime",
  "accept4",
  "signalfd4",
  "eventfd2",
  "epoll_create1",
  "dup3",
  "pipe2",
  "inotify_init1",
  "preadv",
  "pwritev",
  "rt_tgsigqueueinfo",
  "perf_event_open",
  "recvmmsg",
  "fanotify_init",
  "fanotify_mark",
  "prlimit64",
  "name_to_handle_at",
  "open_by_handle_at",
  "clock_adjtime",
  "syncfs",
  "sendmmsg",
  "setns",
  "getcpu",
  "process_vm_readv",
  "process_vm_writev",
  "kcmp",
  "finit_module",
  "sched_setattr",
  "sched_getattr",
  "renameat2",
  "seccomp",
  "getrandom",
  "memfd_create",
  "kexec_file_load",
  "bpf",
  "execveat"
};

int wait_container_process(int pid,char *container_name){
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
    fprintf(ptrace_log,"%s\n",systemcall_table[regs.orig_rax]);
  }

  fclose(ptrace_log);
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
int set_container_name(char *argv[],char *container_name){
  char absolute_path[PATH_MAX];
  realpath(argv[1], absolute_path);
  char *splitted_path[SPLIT_MAX];
  int count = split(absolute_path,"/",splitted_path);
  strncpy(container_name,splitted_path[count],CONTAINER_NAME_MAX);
  return 0;
}

int main(int argc, char *argv[])
{
  check_argument(argc,argv);

  char container_name[CONTAINER_NAME_MAX];
  set_container_name(argv, container_name);
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
      ptrace(PTRACE_TRACEME, 0, NULL, NULL);
      break;
      // parent
    default:
      {
        wait_container_process(pid,container_name);
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

  write_tasks_container_pid(container_cgroups_pid_dir);

  write_pid_max(container_cgroups_pid_dir, 100);

  rc = chdir(argv[1]);
  if(rc < 0){
    printf("chdir Error: %d\n", rc);
  }
  rc = chroot("./");
  if(rc < 0){
    printf("chroot Error: %d\n", rc);
    return(-1);
  }

  rc = mount_proc_fs();
  if(rc < 0){
    return(rc);
  }
  //rc = mount("dev","./dev", "dev", MS_BIND | MS_REC, 0);
  //if(rc < 0){
  //  printf("dev mount Error: %d\n", rc);
  //  return(-1);
  //}

  //rc = mount("devpts","/dev/pts", "devpts", MS_BIND, 0);
  //if(rc < 0){
  //    printf("devpts mount Error: %d\n", rc);
  //    return(-1);
  //}
  //mount -t devpts devpts /dev/pts
  //mount("devpts", "/dev/pts", "devpts",0,0);
  //

  sethostname(container_name,strlen(container_name));

  rc = execl("/bin/sh","/bin/sh", NULL);
  if(rc < 0){
    printf("exec Error: %d\n", rc);
    return(-1);
  }

}

