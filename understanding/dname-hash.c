/* COPYRIGHT (C) 2001 Philippe Elie under GPL license */

/* This test is not intended to be runned automatically, it is rather provided
 * to easier understand how work the dname hash coding stuff in the module */

#include <stdio.h>

#define OP_HASH_MAP_NR	4093

/* The first implementation used this and was called a quadric search but in
 * fact turn back to a linear search after a few iteration.
 * The alogrithm quality depend on the uint_type used. With unsigned short 
 * (16 bits) 75% of the table is covered before a fulltable table occur, with
 * unsigned int (32 bits) less than 25% of the table was covered before a
 * a fulltable occur */

typedef unsigned short uint_type;

void test_hash_dname1(uint_type value) 
{
	uint_type probe = 3;
	uint_type firsthash = value;
	int i;

	for (i = 0 ; i < OP_HASH_MAP_NR; ++i) {
//		printf("%u\t", value);
		value = abs((value + probe) % OP_HASH_MAP_NR);
		probe *= probe;
		if (value == firsthash)
			goto fulltable;
	}

	return;

 fulltable:

	printf("Full table occur at iteration %d for entry %u\n", i, firsthash);
}

/* The current implementation use a modified version of a linear search and
 * is based on:
 *
 * if N is prime, value in [0-N[ and incr = (!value ? 1 : value) then the
 * iteration: value = (value + incr) % N covers the range [0-N[ in N iterations
 * This have advantage against the classical linear search to better sparse
 * the search if many collision occur around a value. No suprise can occur
 * except if the integer type used can not hold the value (N * 2) */
void test_hash_dname2(unsigned int value) 
{
	unsigned int incr = value;
	unsigned int firsthash = value;
	int i;

	if (incr == 0)
		incr = 1;

	for (i = 0 ; i < OP_HASH_MAP_NR ; ++i) {
//		printf("%u\t", value);
		value = (value + incr) % OP_HASH_MAP_NR;
		if (value == firsthash)
			goto fulltable;
	}

	return;

 fulltable:

	printf("Full table occur at iteration %d for entry %u\n", i, firsthash);
}

int main(void)
{
	unsigned int value;

	for (value = 0; value < OP_HASH_MAP_NR ; ++value)
		test_hash_dname2(value);

	return 0;
}
