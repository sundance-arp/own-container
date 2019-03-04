#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>

#include <vector>
#include <filesystem>

#include "commandargument.h"


command_argument init_command_argument(){
  command_argument command_arg;
  command_arg.trace = false;
  command_arg.privilege = false;
  command_arg.bind = false;
  command_arg.bind_paths = std::vector<std::string>();
  command_arg.rootfs_path = std::filesystem::path();
  return command_arg;
}
