#!/bin/bash

basedir=$1

for time in 1000 2000 3000 4000 5000 10000 20000
do pth="$basedir$time"
cd $pth || exit
for file in *
do echo $time $file
echo $file >> stats
cat $file | datamash -H max 1 min 1 mean 1 pstdev 1 max 2 min 2 mean 2 pstdev 2 >> stats
done
done
