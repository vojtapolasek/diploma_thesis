#!/bin/bash



gcc -o benchmark benchmark.c -lcryptsetup
for time in 1000 2000 3000 4000 5000
do mkdir $time
for threads in `seq 1 8`
do echo time $time threads $threads
./benchmark -t $time -p $threads -r 1 -c > $time/stdmem$threads.csv
done
done