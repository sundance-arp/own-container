#!/bin/bash
flag=""
if [ "${1}" = "--debug" ]; then
  flag="${flag} -g"
fi

files="main.c mount-host.c unshare-namespace.c utils.c manage-cgroups.c"

gcc ${flag} ${files} -static -o bin/container
