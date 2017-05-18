#!/bin/sh

EXECUTION_PATH=$1
EXECUTABLE_PATH=$2
NUMBER_OF_INSTANCES=$3

echo "Running: [$EXECUTABLE_PATH] [$NUMBER_OF_INSTANCES] times from [$EXECUTION_PATH]"

for i in $(seq 1 $NUMBER_OF_INSTANCES)
do
	mkdir $i
	cd $i > /dev/null
	echo "Running from $(pwd)"
	nohup ../$EXECUTABLE_PATH &
	cd - > /dev/null
done
