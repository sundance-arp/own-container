#include <string>

struct command_argument
{
  bool trace;
  bool privilege;
  std::string rootfs_path;
};

command_argument init_command_argument();

