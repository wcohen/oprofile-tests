#!/bin/sh

# gen-big-names: create long-named executables to stress the string pool

trap "rm -f gen-big-names-t1.c gen-big-names-t1; exit 1" 1 2 3 15
 
count=16768

echo "int main(){return 0;}" >gen-big-names-t1.c
gcc -o gen-big-names-t1 gen-big-names-t1.c

while [ "$count" != 0 ]
do
	#file=a 
	file="gen-big-names-$RANDOM-longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong";
	file="$file-longlonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglonglong";
	echo "Making file"
	echo "Running file"
	trap "rm -f gen-big-names-t1.c gen-big-names-t1 $file; exit 1" 1 2 3 15
	cp gen-big-names-t1 $file 
	./$file
	rm -f $file
	count=`expr $count - 1`
done
