all: test

.PHONY: all test clean

clean:
	rm -f *~ *.o test

test: hash.c
	gcc -O2 -Wall -fomit-frame-pointer -pedantic -funroll-loops -march=pentiumpro -mcpu=pentiumpro -o $@ $^
