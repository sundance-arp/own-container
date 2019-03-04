#include <string>
#include <vector>
#include <filesystem>


struct command_argument
{
  bool trace;
  bool privilege;
  bool bind;
  std::vector<std::string> bind_paths;
  std::filesystem::path rootfs_path;
};

command_argument init_command_argument();

