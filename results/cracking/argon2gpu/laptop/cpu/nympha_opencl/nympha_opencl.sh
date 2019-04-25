#!/bin/bash
OPTIND=1


count=100
lanes=4
iterations=6
memory=1048576
outputdir="/storage/brno2/home/vojtapolasek/results/nympha_opencl/"

while getopts "hc:M:T:L:o:" opt
do case $opt in
h)
echo "-o for output directory, required"
echo "-c for specifying number of computations, default" $count
echo "-M for memory parameter, default" $memory
echo "-L for number of lanes, default" $lanes
echo "-I for number of iterations, default " $iterations
exit 0
;;
o)
outputdir=$OPTARG
;;
c)
count=$OPTARG
;;
M)
memory=$OPTARG
;;
L)
lanes=$OPTARG
;;
I)
iterations=$OPTARG
;;
?)
echo "invalid argument supplied"
exit 1
;;
esac
done
shift $((OPTIND-1))
[ "${1:-}" = "--" ] && shift

if [ ! -d "$outputdir" ] || [ ! -x "$outputdir" ]
then
echo "invalid or inaccessible output directory"
exit 1
fi

cd $SCRATCHDIR
source /storage/brno2/home/vojtapolasek/.brew_enable
git clone --recursive https://gitlab.com/vojtapolasek/argon2-gpu.git
module add cuda-9.0
cd argon2-gpu
mkdir build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make
ln -s ../data data
for batch in 1 2 4 8 16 32 64 128
do ./argon2-gpu-bench -m opencl -L $lanes -T $iterations -M $memory -s $count -b $batch > $outputdir/$batch.log 2>&1
done
