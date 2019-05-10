#!/bin/bash

inputdir=$1

cd $inputdir || exit 1
for file in *.log
do echo $file >> stats
echo $file
cat $file | datamash mean 1 pstdev 1 >> stats
done
