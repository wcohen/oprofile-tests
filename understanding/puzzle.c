
// This is some real test code for examining oprofile and short functions.
// copyright 2001 Jeff Esper

int func(unsigned long i) {
	static int table[64] = {
		0x20,0x00,0x01,0x0c, 0x02,0x06,  -1,0x0d,
		0x03,  -1,0x07,  -1,   -1,  -1,  -1,0x0e,
		0x0a,0x04,  -1,  -1, 0x08,  -1,  -1,0x19,
		  -1,  -1,  -1,  -1,   -1,0x15,0x1b,0x0f,
		0x1f,0x0b,0x05,  -1,   -1,  -1,  -1,  -1,
		0x09,  -1,  -1,0x18,   -1,  -1,0x14,0x1a,
		0x1e,  -1,  -1,  -1, 0x17,0x17,  -1,0x13,
		0x1d,  -1,0x16,0x12, 0x1c,0x11,0x10,  -1,
	};

	if(!i) return -1;

	return table[ ((i&-i) * 0x0450FBAFl) >> 26];
}

int func2(unsigned long i) {
	static int table[37] = { 
		-1,  0,  1, 26,  2, 23, 27,-1,
		 3, 16, 24, 30, 28, 11, -1, 13,
		 4,  7, 17, -1, 25, 22, 31, 15,
		29, 10, 12,  6, -1, 21, 14,  9,
		 5, 20,  8, 19, 18
	};
	return table[(i&-i) % 37];
}

int main(void) { 
	int i;
	for(i=0; ; i++) { func(i); func2(i); }
	return 0;
}
