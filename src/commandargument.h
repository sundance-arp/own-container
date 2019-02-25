#include <string>
#include <vector>

struct command_argument
{
  bool trace;
  bool privilege;
  bool bind;
  std::vector<std::string> bind_paths;
  std::string rootfs_path;
};

command_argument init_command_argument();

