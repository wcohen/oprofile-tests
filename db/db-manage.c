#define _GNU_SOURCE

#include <stdlib.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>

#include "db.h"

static __inline db_descr * to_descr(root_t * root)
{
	return (db_descr *)(((char *)root->base_memory) + root->offset_descr);
}

static __inline page_t * to_page(root_t * root)
{
	return (page_t *)(((char *)root->base_memory) + root->offset_page);
}

static void init_page(root_t * root, unsigned int from, unsigned to)
{
	/* FIXME: nil_page is not currently a zero value so we need
	 * to initialize it explicitely: perhaps we can consider to
	 * not use the zero page and to use zero as nil_page value
	 * avoiding to touch memory of the mmaped file */
	for ( ; from != to ; ++from) {
		unsigned int count;
		page_t * page;

		/* we can't use page_nr_to_page_ptr here because
		 * page_nr_to_page_ptr can trigger an assertion ! */
		page = &root->page_base[from];
		page->p0 = nil_page;
		for (count = 0 ; count < MAX_PAGE ; ++count) {
			page->page_table[count].child_page = nil_page;
		}
	}
}

page_idx_t add_page(root_t * root)
{
	if (root->descr->current_size >= root->descr->size) {
		unsigned int old_size = root->descr->size;
		unsigned int new_file_size;

		root->descr->size *= 2;

		new_file_size = root->descr->size * sizeof(page_t);
		new_file_size += root->offset_page;

		if (ftruncate(root->fd, new_file_size)) {
			fprintf(stderr, "unable to resize file to %d "
				"length, cause : %s\n",
				new_file_size, strerror(errno));
			exit(EXIT_FAILURE);
		}

		root->base_memory = mremap(root->base_memory,
			 (old_size * sizeof(page_t)) + root->offset_page,
			 new_file_size, MREMAP_MAYMOVE);

		if (root->base_memory == MAP_FAILED) {
			fprintf(stderr, "add_page() mremap failure "
				"cause: %s\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

		root->descr = to_descr(root);
		root->page_base = to_page(root);

		init_page(root, old_size, root->descr->size);
	}

	return (page_idx_t)root->descr->current_size++;
}

/* the default number of page, calculated to fit in 4096 bytes */
#define DEFAULT_PAGE_NR(offset_page)			\
	(4096 - offset_page) / sizeof(page_t) ?		\
	(4096 - offset_page) / sizeof(page_t) : 1 

void db_open(root_t * root, const char * filename, unsigned int offset_page,
	     unsigned int offset_descr)
{
	struct stat stat_buf;
	unsigned int nr_page;

	memset(root, '\0', sizeof(root_t));

	root->offset_page = offset_page;
	root->offset_descr = offset_descr;

	root->fd = open(filename, O_RDWR | O_CREAT, 0644);
	if (root->fd < 0) {
		fprintf(stderr, "db_open() fail to open %s case: %s\n",
			filename, strerror(errno));
		exit(EXIT_FAILURE);
	}

	/* if the length of file is zero we have created it so we must grow
	 * it. Can we grow it lazilly or must the lazilly things handled
	 * by caller ? */
	if (fstat(root->fd, &stat_buf)) {
		fprintf(stderr, "unable to stat %s cause %s\n",
			filename, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (stat_buf.st_size == 0) {
		unsigned int file_size;

		nr_page = DEFAULT_PAGE_NR(root->offset_page);

		file_size = offset_page + (nr_page * sizeof(page_t));
		if (ftruncate(root->fd, file_size)) {
			fprintf(stderr, "unable to resize file %s to %d "
				"length, cause : %s\n",
				filename, file_size, strerror(errno));
		}
	} else {
		nr_page = (stat_buf.st_size - offset_page) / sizeof(page_t);

		if (nr_page != root->descr->size) {
			fprintf(stderr, "nr_page != root->descr->size\n");
			exit(EXIT_FAILURE);
		}
	}

	root->base_memory = 
		mmap(0, (nr_page * sizeof(page_t)) + root->offset_page,
		     PROT_READ | PROT_WRITE, MAP_SHARED, root->fd, 0);

	if (root->base_memory == MAP_FAILED) {
		fprintf(stderr, "add_page() mmap failure "
			"cause: %s\n", strerror(errno));
		exit(EXIT_FAILURE);
	}


	root->descr = to_descr(root);
	root->page_base = to_page(root);

	if (stat_buf.st_size == 0) {
		root->descr->size = nr_page;
		root->descr->current_size = 0;
		root->descr->root_idx = nil_page;

		init_page(root, 0, root->descr->size);
	}
}

void db_close(root_t * root)
{
	if (root->page_base)
		munmap(root->base_memory,
		     (root->descr->size * sizeof(page_t)) + root->offset_page);

	if (root->fd != -1)
		close(root->fd);
}

