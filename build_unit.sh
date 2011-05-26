#!/usr/bin/env sh
# http://gcc.gnu.org/onlinedocs/gccint/Plugins.html
g++ -fplugin=$PWD/xkaapi.so -fplugin-arg-xkaapi-key=val unit/class.cc
