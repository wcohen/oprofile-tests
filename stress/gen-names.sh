#!/bin/sh

# gen-names: create executables to stress the hash map

trap "rm -f gen-names-t1.c gen-names-t1; exit 1" 1 2 3 15
 
count=16768

echo "int main(){return 0;}" >gen-names-t1.c
gcc -o gen-names-t1 gen-names-t1.c

while [ "$count" != 0 ]
do
	file="gen-names-$RANDOM";
	echo "Making $file"
	echo "Running $file"
	trap "rm -f gen-names-t1.c gen-names-t1 $file; exit 1" 1 2 3 15
	cp gen-names-t1 $file 
	./$file
	rm -f $file
	count=`expr $count - 1`
done
