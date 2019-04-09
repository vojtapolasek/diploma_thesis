#!/bin/bash

cd cryptsetup-2.1.0/
make clean
./configure --disable-udev --disable-blkid CPPFLAGS='-I../usr/include/ -O3 -march=x86-64' LDFLAGS='-L../usr/lib/x86_64-linux-gnu/'
make
cd ..
if [ "$1" = "-b" ]; then
cd cpulimit
make clean
make
cd ..
gcc -O3 -march=x86-64 -Wl,-Lcryptsetup-2.1.0/.libs/ -o benchmark -I cryptsetup-2.1.0/lib/ -Wl,-rpath=cryptsetup-2.1.0/.libs/ benchmark.c -lcryptsetup -pthread
fi
if [ "$1" = "-c" ]; then
gcc -O3 -march=x86-64 -Wl,-Lcryptsetup-2.1.0/.libs/ -o cracker -I cryptsetup-2.1.0/lib/ -Wl,-rpath=cryptsetup-2.1.0/.libs/ cracker.c -lcryptsetup
gcc -O3 -march=x86-64 -Wl,-Lcryptsetup-2.1.0/.libs/ -o cracker_multithreaded -I cryptsetup-2.1.0/lib/ -Wl,-rpath=cryptsetup-2.1.0/.libs/ cracker_multithreaded.c -lcryptsetup -pthread
fi