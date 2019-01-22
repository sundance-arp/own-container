#!/bin/bash
project_dir=$(cd $(dirname $0); pwd)
src_dir="${project_dir}/src"

flag=""
if [ "${1}" = "--debug" ]; then
  flag="${flag} -g"
fi

files="main.cpp mount.c namespace.c utils.c cgroups.c ptrace.c"

$(cd ${src_dir}; g++ ${flag} ${files} -static -o ${project_dir}/bin/container)
sudo setcap CAP_DAC_OVERRIDE,CAP_SYS_CHROOT,CAP_SYS_ADMIN=eip ${project_dir}/bin/container

