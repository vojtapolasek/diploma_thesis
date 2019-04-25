#!/bin/bash

workingdir=$SCRATCHDIR
outputdir="/storage/brno2/home/vojtapolasek/results/konos_all_rpi/"
maxbatch=1
lanes=4
iterations=4
memory=95297

eval $(/storage/brno2/home/vojtapolasek/.linuxbrew/bin/brew shellenv)
uname -a > $outputdir/uname.log
cd $workingdir || exit 1
module add cuda-9.0
git clone --recursive https://gitlab.com/omos/argon2-gpu.git
#standard kernel
cd argon2-gpu
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
#finding max batch size
while :
do ./argon2-gpu-bench -m cuda -L $lanes -T $iterations -M $memory -s 1 -b $maxbatch -p
if [ "$?" -ne 0  ]
then let maxbatch=$maxbatch-1
break
fi
let maxbatch=$maxbatch+1
done
echo $maxbatch
max=$maxbatch
for maxbatch in `seq 1 $max`
do ./argon2-gpu-bench -m cuda -L $lanes -T $iterations -M $memory -s 100 -b $maxbatch 2>&1 > $outputdir/master_$maxbatch.log
./argon2-gpu-bench -m cuda -L $lanes -T $iterations -M $memory -s 100 -b $maxbatch -p 2>&1 > $outputdir/master_precomputes$maxbatch.log
./argon2-gpu-bench -m cuda -L $lanes -T $iterations -M $memory -s 100 -b $maxbatch -k oneshot 2>&1 > $outputdir/master_oneshot_$maxbatch.log
./argon2-gpu-bench -m cuda -L $lanes -T $iterations -M $memory -s 100 -b $maxbatch -k oneshot -p 2>&1 > $outputdir/master_oneshot_precomputes_$maxbatch.log

#warp-shuffle
cd ..
rm -rf build
git checkout -b warp-shuffle
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
./argon2-gpu-bench -m cuda -L $lanes -T $iterations -M $memory -s 100 -b $maxbatch 2>&1 > $outputdir/warpshuffle_$maxbatch.log
./argon2-gpu-bench -m cuda -L $lanes -T $iterations -M $memory -s 100 -b $maxbatch -p 2>&1 > $outputdir/warpshuffle_precomputes_$maxbatch.log
./argon2-gpu-bench -m cuda -L $lanes -T $iterations -M $memory -s 100 -b $maxbatch -k oneshot 2>&1 > $outputdir/warpshuffle_oneshot_$maxbatch.log
./argon2-gpu-bench -m cuda -L $lanes -T $iterations -M $memory -s 100 -b $maxbatch -k oneshot -p 2>&1 > $outputdir/warpshuffle_oneshot_precomputes_$maxbatch.log
#warp-shuffle-shared
cd ..
rm -rf build
git checkout -b warp-shuffle-shared
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
./argon2-gpu-bench -m cuda -L $lanes -T $iterations -M $memory -s 100 -b $maxbatch 2>&1 > $outputdir/warpshuffleshared_$maxbatch.log
./argon2-gpu-bench -m cuda -L $lanes -T $iterations -M $memory -s 100 -b $maxbatch -p 2>&1 > $outputdir/warpshuffleshared_precomputes_$maxbatch.log
./argon2-gpu-bench -m cuda -L $lanes -T $iterations -M $memory -s 100 -b $maxbatch -k oneshot 2>&1 > $outputdir/warpshuffleshared_oneshot_$maxbatch.log
./argon2-gpu-bench -m cuda -L $lanes -T $iterations -M $memory -s 100 -b $maxbatch -k oneshot -p 2>&1 > $outputdir/warpshuffleshared_oneshot_precomputes_$maxbatch.log
done
