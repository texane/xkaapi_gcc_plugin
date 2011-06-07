#!/usr/bin/env sh
# http://gcc.gnu.org/onlinedocs/gccint/Plugins.html
rm *xkaapi_pass*
g++ -fdump-tree-all -fplugin=$PWD/xkaapi.so -fplugin-arg-xkaapi-key=val unit/scalar_arg.c
