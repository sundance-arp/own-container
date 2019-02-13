#include <map>

#ifndef COMMANDARGUMENT_H
#define COMMANDARGUMENT_H
#include "commandargument.h"
#endif


int get_clone_flags(command_argument command_arg);
int set_container_arg(struct container_arg *ca, command_argument command_arg,char *container_name);
void setgroups_control(int pid);
void map_id(const char *file, int from, int to);


struct container_arg
{
  command_argument command_arg;
  char *container_name;
  int    pipe_fd[2];
};
