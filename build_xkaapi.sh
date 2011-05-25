#!/usr/bin/env sh
# http://gcc.gnu.org/onlinedocs/gccint/Plugins.html

# GCC=$HOME/install/bin/gcc
gcc \
-I/usr/lib/gcc/i486-linux-gnu/4.5.3/plugin/ \
-I/usr/lib/gcc/i486-linux-gnu/4.5.3/plugin/include \
-shared -fPIC -O2 \
xkaapi.c -o xkaapi.so
