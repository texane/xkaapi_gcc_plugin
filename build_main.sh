#!/usr/bin/env sh
# http://gcc.gnu.org/onlinedocs/gccint/Plugins.html
gcc -fplugin=$PWD/xkaapi.so -fplugin-arg-xkaapi-key=val -o main main.c
