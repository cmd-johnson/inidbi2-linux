#!/usr/bin/env sh
gcc -Wall -Werror -shared -fPIC -m32 -o inidbi2.so src/inidbi2.cpp
gcc -Wall -Werror -shared -fPIC -m64 -o inidbi2.x86-64.so src/inidbi2.cpp
