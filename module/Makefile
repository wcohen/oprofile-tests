all: test

.PHONY: all test clean

clean:
	rm -f *~ *.o test

test: hash.c
	gcc -O2 -g -Wall -fomit-frame-pointer -funroll-loops -march=pentiumpro -mcpu=pentiumpro -o $@ $^
