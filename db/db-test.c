#include <sys/resource.h>
#include <stdlib.h>
#include <stdio.h>

#include "db.h"

static int nr_error;

static double user_time()
{
  struct rusage  usage;

  getrusage(RUSAGE_SELF, &usage);

  return usage.ru_utime.tv_sec + (usage.ru_utime.tv_usec / 1000000.0);
}

/* create nr item randomly created with nr_unique_item distinct items */
static void speed_test(unsigned int nr_item, unsigned int nr_unique_item)
{
	int i;
	double begin, end;
	root_t root;

	init_root(&root);
	begin = user_time();
	for (i = 0 ; i < nr_item ; ++i) {
		insert(&root, random() % nr_unique_item, 1);
	}
	end = user_time();
	delete_root(&root);

	fprintf(stderr, "nr item: %d, unique item: %d, elapsed: %f\n",
		nr_item, nr_unique_item, end - begin);
}

static void do_speed_test(void)
{
	int i, j;

	for (i = 1000 ; i <= 1000000 ; i *= 10) {
		for (j = 100 ; j <= i / 10 ; j *= 10) {
			speed_test(i, j);
		}
	}
}

static int test(unsigned int nr_item, unsigned int nr_unique_item)
{
	int i;
	root_t root;
	int ret;

	init_root(&root);

	for (i = 0 ; i < nr_item ; ++i) {
		insert(&root, random() % nr_unique_item, 1);
	}

	ret = check_tree(&root);

	delete_root(&root);

	return ret;
}

static void do_test(void)
{
	int i, j;

	for (i = 1000 ; i <= 1000000 ; i *= 10) {
		for (j = 100 ; j <= i / 10 ; j *= 10) {
			if (test(i, j)) {
				printf("%s:%d failure for %d %d\n",
				       __FILE__, __LINE__, i, j);
				nr_error++;
			} else {
				printf("test() ok %d %d\n", i, j);
			}
		}
	}
}


static unsigned int range_first, range_last;
static unsigned int last_key_found;
static void call_back(unsigned int key, unsigned int info, void * data)
{
	if (key <= last_key_found) {
		printf("%x %x\n", key, last_key_found);
		nr_error++;
	}

	if (key < range_first || key >= range_last)
 {
		printf("%x %x %x\n", key, range_first, range_last);
		nr_error++;
	}

	last_key_found = key;
}

static int callback_test(int nr_item, int nr_unique_item)
{
	int i;
	root_t root;
	unsigned int first_key, last_key;
	unsigned int old_nr_error = nr_error;

	init_root(&root);

	for (i = 0 ; i < nr_item ; ++i) {
		insert(&root, (random() % nr_unique_item) + 1, 1);
	}


	last_key = (unsigned int)-1;
	first_key = 0;
	
	for ( ; first_key < last_key ; last_key /= 2) {

		last_key_found = first_key == 0 ? first_key : first_key - 1;
		range_first = first_key;
		range_last = last_key;

		travel(&root, first_key, last_key, call_back, 0);

		first_key = first_key == 0 ? 1 : first_key * 2;
	}

	last_key_found = 0;
	range_first = 0;
	range_last = -1;
	
	travel(&root, range_first, range_last, call_back, 0);

	delete_root(&root);

	return old_nr_error == nr_error ? 0 : 1;
}

static void do_callback_test(void)
{
	int i, j;
	for (i = 1000 ; i <= 1000000 ; i *= 10) {
		for (j = 100 ; j <= i / 10 ; j *= 10)
			if (callback_test(i, j)) {
				printf("%s:%d failure for %d %d\n",
				       __FILE__, __LINE__, i, j);
				nr_error++;
			} else {
				printf("callback_test() ok %d %d\n", i, j);
			}
	}
}

int main(void)
{
	do_test();

	do_callback_test();

	do_speed_test();

	if (nr_error) {
		printf("%d error occured\n", nr_error);
	} else {
		printf("no error occur\n");
	}

	return 0;
}
