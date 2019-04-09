#!/bin/bash

count=100
lanes=4
iterations=6
memory=1048576


cd $SCRATCHDIR
source /storage/brno2/home/vojtapolasek/.brew_enable
git clone --recursive https://gitlab.com/vojtapolasek/argon2-gpu.git
module add cuda-9.0
cd argon2-gpu
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
for batch in 1 2 4 8 16 32 64 128 256 512 1024
do ./argon2-gpu-bench -m cpu -L $lanes -T $iterations -M $memory -s $count -b $batch > /storage/brno2/home/vojtapolasek/results/$batch.log 2>&1
done