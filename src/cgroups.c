#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <limits.h>

int create_container_cgroups_directory(char* path){
  return mkdir(path,0755);
}

int write_tasks_container_pid(char* cgroup_subsystem_path){
  // cgroupへpidの書き込み
  FILE *cgroups_file;
  char tasks_path[PATH_MAX];
  snprintf(tasks_path, PATH_MAX, "%s/tasks", cgroup_subsystem_path);
  cgroups_file = fopen(tasks_path,"a+");
  if (cgroups_file == NULL) {
    printf("cgroup task file open Error\n");
    exit(1);
  }
  fclose(cgroups_file);
  return 0;
}

int write_pid_max(char* cgroups_pids_path, int max_pid){
  FILE *cgroups_file;
  char pids_max_path[PATH_MAX];
  snprintf(pids_max_path, PATH_MAX, "%s/pids.max", cgroups_pids_path);
  cgroups_file = fopen(pids_max_path,"a+");

  fprintf(cgroups_file,"%d\n",max_pid);
  fclose(cgroups_file);
  return 0;
}
