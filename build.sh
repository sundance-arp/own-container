#!/bin/bash -eux
project_dir=$(cd $(dirname $0); pwd)
src_dir="${project_dir}/src"

flag="-std=c++17 -lstdc++fs -Wall"
set +u
if [ "${1}" = "--debug" ]; then
  flag="${flag} -g"
fi
set -u

files="main.cc mount.cc namespace.cc utils.c cgroups.c ptrace.c commandargument.cc"

$(cd ${src_dir}; g++ ${files} ${flag}  -static -o ${project_dir}/bin/container)
sudo setcap CAP_DAC_OVERRIDE,CAP_SYS_CHROOT,CAP_SYS_ADMIN=eip ${project_dir}/bin/container

