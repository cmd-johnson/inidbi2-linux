#!/usr/bin/env sh
gcc -Wall -Werror -shared -fPIC -m32 -O3 -o inidbi2.so src/inidbi2.cpp
gcc -Wall -Werror -shared -fPIC -m64 -O3 -o inidbi2.x86-64.so src/inidbi2.cpp
