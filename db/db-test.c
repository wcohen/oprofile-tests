#include <sys/resource.h>
//#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "db.h"

static double user_time()
{
  struct rusage  usage;

  getrusage(RUSAGE_SELF, &usage);

  return usage.ru_utime.tv_sec + (usage.ru_utime.tv_usec / 1000000.0);
}

int main()
{
	int i;
	double begin, end;
	root_t root;

	init_root(&root);

#if 0
	insert(&root, 16, 1);
	insert(&root, 32, 1);
	insert(&root, 48, 1);
	insert(&root, 64, 1);
	insert(&root, 80, 1);
	insert(&root, 96, 1);
	insert(&root, 100, 1);
	insert(&root, 9, 1);
//	insert(&root, 11, 1);
//	insert(&root, 13, 1);
//	insert(&root, 1, 1);
#else
	begin = user_time();
	for (i = 0 ; i < 1000000 ; ++i) {
		insert(&root, random() % 10000, 1);
	}
	end = user_time();

	fprintf(stderr, "%f\n", end - begin);
#endif

	if (check_tree(&root, root.root, 0u))
		fprintf(stderr, "test failure\n");
	else
		fprintf(stderr, "test ok\n");

	// recursively travel in the btree and check than all thing are right
//	display_tree(&root, root.root);

	delete_root(&root);

	return 0;
}
