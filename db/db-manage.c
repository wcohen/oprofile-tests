#include <stdlib.h>
#include <stdio.h>

#include "db.h"

page_idx_t add_page(root_t * root)
{
	if (root->current_size >= root->size) {
		
		unsigned int old_size = root->size;
		unsigned int pos;

		root->size = root->size ? root->size * 2 : 1;
		root->base_area =
			realloc(root->base_area, root->size *sizeof(page_t));

		memset(&root->base_area[old_size], '\0',
		       (old_size == 0 ? 1 : old_size) * sizeof(page_t));

		for (pos = old_size ; pos < root->size ; ++pos) {
			unsigned int count;
			page_t * page;

			/* we can't use page_nr_to_page_ptr here because
			 * page_nr_to_page trigger an assertion ! */
			page = &root->base_area[pos];
//			page = page_nr_to_page_ptr(root, pos);
			page->p0 = (page_idx_t)-1;
			for (count = 0 ; count < MAX_PAGE ; ++count) {
				page->page_table[count].child_page = (page_idx_t)-1;
			}
		}
	}

	return (page_idx_t)root->current_size++;
}

void init_root(root_t * root)
{
	memset(root, '\0', sizeof(root_t));
}

void delete_root(root_t * root)
{
	if (root->base_area)
		free(root->base_area);
}

void display_tree(const root_t * root, page_idx_t page_nr)
{
#if 1
	unsigned int i;
	page_t * page;

	page = page_nr_to_page_ptr(root, page_nr);

	if (page->p0 != nil_page)
		display_tree(root, page->p0);

	for (i = 0 ; i < page->count ; ++i) {
		printf("%d\t", page->page_table[i].key);
		if (page->page_table[i].child_page != nil_page)
			display_tree(root, page->page_table[i].child_page);
	}
#else
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
#endif
}

int check_tree(const root_t * root, page_idx_t page_nr, unsigned int last)
{
	unsigned int i;
	page_t * page;

	page = page_nr_to_page_ptr(root, page_nr);

	if (page->p0 != nil_page)
		check_tree(root, page->p0, last);

	for (i = 0 ; i < page->count ; ++i) {
		if (page->page_table[i].key <= last)
			return 1;
		last = page->page_table[i].key;
		if (page->page_table[i].child_page != nil_page)
			if (check_tree(root, page->page_table[i].child_page, last))
				return 1;
	}

	return 0;
}
