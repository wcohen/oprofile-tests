#include <stdio.h>
#include <stdlib.h>

#include "db.h"


static void do_display_tree(const root_t * root, page_idx_t page_idx)
{
	unsigned int i;
	page_t * page;

	if (page_idx == nil_page)
		return;

	page = page_nr_to_page_ptr(root, page_idx);

	do_display_tree(root, page->p0);

	for (i = 0 ; i < page->count ; ++i) {
		printf("%d\t", page->page_table[i].key);
		do_display_tree(root, page->page_table[i].child_page);
	}
}

void display_tree(const root_t * root)
{
	do_display_tree(root, root->root);
}

static void do_raw_display_tree(const root_t * root, int page_idx)
{
	unsigned int i;
	printf("root %d\n", root->root);
	for (i = 0 ; i < root->current_size ; ++i) {
		page_t * page;
		int j;

		page = page_nr_to_page_ptr(root, i);
		printf("p0: %d\n", page->p0);
		for (j = 0 ; j < page->count ; ++j) {
			printf("(%d, %d, %d)\n",
			       page->page_table[j].key,
			       page->page_table[j].info,
			       page->page_table[j].child_page);
		}
		printf("\n");
	}
}

void raw_display_tree(const root_t * root)
{
	do_raw_display_tree(root, root->root);
}

static int do_check_page_pointer(const root_t * root, unsigned int page_idx,
				  int * viewed_page)
{
	int ret;
	int i;
	const page_t * page;

	if (page_idx == nil_page)
		return 0;

	if (page_idx >= root->current_size) {
		printf("%s:%d invalid page number, max is %d page_nr is %d\n",
		       __FILE__, __LINE__, root->current_size, page_idx);
		return 1;
	}

	if (viewed_page[page_idx]) {
		printf("%s:%d child page number duplicated %d\n",
		       __FILE__, __LINE__, page_idx);
		return 1;
	}

	viewed_page[page_idx] = 1;

	page = page_nr_to_page_ptr(root, page_idx);

	ret = do_check_page_pointer(root, page->p0, viewed_page);

	for (i = 0 ; i < page->count ; ++i) {
		ret |= do_check_page_pointer(root,
					     page->page_table[i].child_page,
					     viewed_page);
	}

	/* this is not a bug, item at pos >= page->count are in an undefined
	 * state */
/*
	for ( ; i < MAX_PAGE ; ++i) {
		if (page->page_table[i].child_page != nil_page) {
			ret = 1;
		}
	}
*/

	return ret;
}

int check_page_pointer(const root_t * root)
{
	int ret;
	int * viewed_page;

	if (root->current_size > root->size) {
		printf("%s:%d invalid current size %d, %d\n",
		       __FILE__, __LINE__, root->current_size, root->size);
	}

	viewed_page = calloc(root->current_size, sizeof(int));

	ret = do_check_page_pointer(root, root->root, viewed_page);

	free(viewed_page);

	return ret;
}

static int do_check_tree(const root_t * root, page_idx_t page_nr,
			 unsigned int last)
{
	unsigned int i;
	const page_t * page;

	page = page_nr_to_page_ptr(root, page_nr);

	if (page->p0 != nil_page)
		do_check_tree(root, page->p0, last);

	for (i = 0 ; i < page->count ; ++i) {
		if (page->page_table[i].key <= last)
			return 1;
		last = page->page_table[i].key;
		if (page->page_table[i].child_page != nil_page)
			if (do_check_tree(root, page->page_table[i].child_page,
				       last))
				return 1;
	}

	return 0;
}

int check_tree(const root_t * root)
{
	int ret = check_page_pointer(root);
	if (!ret)
		ret = do_check_tree(root, root->root, 0u);

	return ret;
}
