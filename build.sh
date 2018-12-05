#!/bin/bash
flag=""
if [ "${1}" = "--debug" ]; then
  flag="${flag} -g"
fi

files="main.c mount.c unshare-namespace.c utils.c cgroups.c"

gcc ${flag} ${files} -static -o bin/container
