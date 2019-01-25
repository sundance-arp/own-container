#include <map>


int get_clone_flags(std::map<std::string,const char *> command_options);
int unshare_namespace();
void setgroups_control(int pid);
void map_id(const char *file, int from, int to);
