#include <stdio.h>

#include "db.h"

static void do_travel(const root_t * root, unsigned int page_idx,
		      unsigned int first, unsigned int last,
		      travel_callback callback, void * data)
{
	int pos;
	page_t * page;
	page_idx_t child_page_idx = nil_page;

	if (page_idx == nil_page)
		return;

	page = page_nr_to_page_ptr(root, page_idx);

	/* for now I use a linear search until we choose the underlined page
	 * size (medium or little) */
	for (pos = 0 ; pos < page->count ; ++pos) {
		if (page->page_table[pos].key >= first)
			break;
	}

	if (pos == page->count || page->page_table[pos].key != first) {
		if (pos == 0) {
			child_page_idx = page->p0;
		} else {
			child_page_idx = page->page_table[pos - 1].child_page;
		}
	}

	do_travel(root, child_page_idx, first, last, callback, data);

	for ( ; pos < page->count && page->page_table[pos].key < last; ++pos) {

		callback(page->page_table[pos].key, page->page_table[pos].info,
			 data);

		do_travel(root, page->page_table[pos].child_page,
			  first, last, callback, data);
	}
}

void travel(const root_t * root, unsigned int first, unsigned int last,
	    travel_callback callback, void * data)
{
	do_travel(root, root->descr->root_idx, first, last, callback, data);
}
