#!/bin/bash

for time in 1000 2000 3000 4000 5000 10000 20000
do mkdir $time
for threads in 1 2 3 4
do echo time $time threads $threads
./benchmark -t $time -p $threads -r 100 -c -m 3145728 > $time/stdmem$threads.csv
done
done
