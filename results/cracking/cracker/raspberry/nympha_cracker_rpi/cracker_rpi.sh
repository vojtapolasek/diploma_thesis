#!/bin/bash

workdir="$SCRATCHDIR"
outputdir="/storage/brno2/home/vojtapolasek/results/nympha_cracker_rpi/"

cd $workdir || exit 1
cp /storage/brno2/home/vojtapolasek/cryptsetup-2.1.0.tar.xz .
cp /storage/brno2/home/vojtapolasek/cracker.c .
cp -r /storage/brno2/home/vojtapolasek/usr .
cp -r /storage/brno2/home/vojtapolasek/lib .
cp /storage/brno2/home/vojtapolasek/core.c.patch .
cp /storage/brno2/home/vojtapolasek/test.disk.rpi .
cp /storage/brno2/home/vojtapolasek/dict.txt .
tar xf cryptsetup-2.1.0.tar.xz
cd cryptsetup-2.1.0/
make clean
patch lib/crypto_backend/argon2/core.c ../core.c.patch
./configure --disable-udev --disable-blkid CPPFLAGS='-I../usr/include/ -march=native' LDFLAGS='-L../usr/lib/x86_64-linux-gnu/' --enable-internal-sse-argon2
make
cd ..
gcc -O3 -march=native -Wl,-Lcryptsetup-2.1.0/.libs/ -o cracker -I cryptsetup-2.1.0/lib/ -Wl,-rpath=cryptsetup-2.1.0/.libs/ cracker.c -lcryptsetup -lpthread
echo "copying"
for n in `seq 0 128`
do cp test.disk.rpi test.disk$n
done
echo "copying done"
for p in 1 2 4 8 16 32 64 128
do ./cracker -t luks2 -i test.disk -p dict.txt -T $p > $outputdir/$p.log
done
