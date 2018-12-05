#!/bin/bash
flag=""
if [ "${1}" = "--debug" ]; then
  flag="${flag} -g"
fi

files="main.c mount.c namespace.c utils.c cgroups.c ptrace.c"

gcc ${flag} ${files} -static -o bin/container
