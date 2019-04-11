#!/bin/bash


rm -rf cryptsetup-2.1.0/
tar xf cryptsetup-2.1.0.tar.xz
if [ "$1" = "-b" ]; then
sudo apt-get update
sudo apt-get install libssl-dev libpopt-dev uuid-dev libdevmapper-dev libjson-c-dev
cd cryptsetup-2.1.0/
make clean
patch lib/utils.c ../utils.c.patch
./configure --disable-udev --disable-blkid
read -n1 -r -p "waiting"
make
cd ..
gcc -O3 -march=native -Wl,-Lcryptsetup-2.1.0/.libs/ -o benchmark -I cryptsetup-2.1.0/lib/ -Wl,-rpath=cryptsetup-2.1.0/.libs/ benchmark.c -lcryptsetup
fi
if [ "$1" = "-c" ]; then
cd cryptsetup-2.1.0/
make clean
patch lib/crypto_backend/argon2/core.c ../core.c.patch
./configure --disable-udev --disable-blkid CPPFLAGS='-I../usr/include/ -march=native' LDFLAGS='-L../usr/lib/x86_64-linux-gnu/' --enable-internal-sse-argon2
make
cd ..
gcc -O3 -march=native -Wl,-Lcryptsetup-2.1.0/.libs/ -o cracker -I cryptsetup-2.1.0/lib/ -Wl,-rpath=cryptsetup-2.1.0/.libs/ cracker.c -lcryptsetup -lpthread
fi
