#!/bin/bash
project_dir=$(cd $(dirname $0); pwd)
src_dir="${project_dir}/src"

flag=""
if [ "${1}" = "--debug" ]; then
  flag="${flag} -g"
fi

files="main.c mount.c namespace.c utils.c cgroups.c ptrace.c"

$(cd ${src_dir}; gcc ${flag} ${files} -static -o ${project_dir}/bin/container)

