#!/bin/bash

workdir="$SCRATCHDIR"
outputdir="" #fill in

cd $workdir || exit 1
cp /storage/brno2/home/vojtapolasek/cryptsetup-2.1.0.tar.xz .
cp /storage/brno2/home/vojtapolasek/cracker.c .
cp -r /storage/brno2/home/vojtapolasek/usr .
cp -r /storage/brno2/home/vojtapolasek/lib .
cp /storage/brno2/home/vojtapolasek/core.c.patch .
cp /storage/brno2/home/vojtapolasek/test.disk .
cp /storage/brno2/home/vojtapolasek/dict.txt .
tar xf cryptsetup-2.1.0.tar.xz
cd cryptsetup-2.1.0/
make clean
patch lib/crypto_backend/argon2/core.c ../core.c.patch
./configure --disable-udev --disable-blkid CPPFLAGS='-I../usr/include/ -march=native' LDFLAGS='-L../usr/lib/x86_64-linux-gnu/' --enable-internal-sse-argon2
make
cd ..
gcc -O3 -march=native -Wl,-Lcryptsetup-2.1.0/.libs/ -o cracker -I cryptsetup-2.1.0/lib/ -Wl,-rpath=cryptsetup-2.1.0/.libs/ cracker.c -lcryptsetup -lpthread

for p in 1 2 4 8 16 32 64 128 256 512
do ./cracker -t luks2 -i test.disk -p dict.txt -T $p 2>&1 > $outputdir/$p.log
done
