CFLAGS=-O2 -Wall -pedantic -g -fomit-frame-pointer
CFLAGS+=-DNDEBUG

.PHONY: db-test clean

all: db-test

clean:
	rm -f *.o *~ db-test

db-test: db-insert.o db-manage.o db-test.o
	$(CC) $(CFLAGS) $^ -o $@

ALL_SOURCES = db-insert.c db-manage.c db-test.c

include Rules.make
