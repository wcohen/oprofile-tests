#include <stdlib.h>

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
			 * page_nr_to_page_ptr can trigger an assertion ! */
			page = &root->base_area[pos];
//			page = page_nr_to_page_ptr(root, pos);
			page->p0 = nil_page;
			for (count = 0 ; count < MAX_PAGE ; ++count) {
				page->page_table[count].child_page = nil_page;
			}
		}
	}

	return (page_idx_t)root->current_size++;
}

/* TODO: this as nothing todo here ? */
void init_root(root_t * root)
{
	memset(root, '\0', sizeof(root_t));
}

void delete_root(root_t * root)
{
	if (root->base_area)
		free(root->base_area);
}

