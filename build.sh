#!/bin/bash
flag=""
if [ "${1}" = "--debug" ]; then
  flag="${flag} -g"
fi
gcc ${flag} main.c -static -o bin/container
