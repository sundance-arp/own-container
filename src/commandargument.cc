#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>

#include "commandargument.h"


command_argument init_command_argument(){
  command_argument command_arg;
  command_arg.trace = false;
  command_arg.privilege = false;
  command_arg.rootfs_path = "";
  return command_arg;
}
