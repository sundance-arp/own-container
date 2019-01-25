#include <map>


int get_clone_flags(std::map<std::string,const char *> command_options);
int set_container_arg(struct container_arg *ca, std::map<std::string,const char *> command_options,char *container_name);
void setgroups_control(int pid);
void map_id(const char *file, int from, int to);


struct container_arg
{
  std::map<std::string,const char *> command_options;
  char *container_name;
  int    pipe_fd[2];
};
