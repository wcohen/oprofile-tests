#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>

/*
A very simple module for testing the icky
offset stuff for the kernel. You /should/ see
that do_c() has the lion's share, then do_b(),
then do_a(). the unused_* stuff shouldn't appear
at all.
*/ 
 
void unused_function1()
{
	int i;
	int j;

	for (i = 0 ; i < j; i++) {
		i = j;
		j = i = j;
	}

	i = 4;
	if (i == 4) j = 2;
	if (j == 2) i = j / 2;
	j = 4 /343;
}
 
void do_a()
{
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
{ 
	int i[400];
	int j = 0;

	for (; j < 2000000; j++ ) {
		i[j % 400] = j;
	}
} 

	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
}

void unused_function2()
{
	int i;
	int j;

	for (i = 0 ; i < j; i++) {
		i = j;
		j = i = j;
	}

	i = 4;
	if (i == 4) j = 2;
	if (j == 2) i = j / 2;
	j = 4 /343;
}
 
void do_b()
{
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
{ 
	int i[400];
	int j = 0;

	for (; j < 20000000; j++ ) {
		i[j % 400] = j;
	}
} 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
} 

void unused_function3()
{
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
{ 
	int i;
	int j;

	for (i = 0 ; i < j; i++) {
		i = j;
		j = i = j;
	}

	i = 4;
	if (i == 4) j = 2;
	if (j == 2) i = j / 2;
	j = 4 /343;
} 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
}
 
void do_c()
{
 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
{ 
	int i[400];
	int j = 0;

	for (; j < 200000000; j++ ) {
		i[j % 400] = j;
	} 
} 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
	asm volatile("nop"); 
} 

static int __init init_lart(void)
{
	do_a(); 
	do_b();
	do_c(); 
	return 0;
}


static void __exit cleanup_lart(void)
{
}

module_init(init_lart);
module_exit(cleanup_lart);
