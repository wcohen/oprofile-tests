#include <assert.h>

#include "db.h"

static void copy_item(value_t * dest, const value_t * src,
		      unsigned int nr_item)
{
	unsigned int i;
	for (i = 0 ; i < nr_item ; ++i) {
		dest[i] = src[i];
	}
}

static void copy_item_backward(value_t * dest, value_t * src,
			       unsigned int nr_item)
{
	unsigned int i;
	for (i = nr_item ; i-- > 0 ; ) {
		dest[i] = src[i];
	}
}

static void split_page(root_t * root, 
		       page_idx_t page_idx, page_idx_t new_page_idx,
		       value_t * value, value_t * excess_elt, unsigned int pos)
{
	page_t * page, * new_page;

	new_page = page_nr_to_page_ptr(root, new_page_idx);
	page = page_nr_to_page_ptr(root, page_idx);

	/* split page in two piece of equal size */

	if (pos < MIN_PAGE) {
		/* save the pivot. */
		*excess_elt = page->page_table[MIN_PAGE - 1];

		/* split-up the page. */
		copy_item(&new_page->page_table[0],
			  &page->page_table[MIN_PAGE],
			  MIN_PAGE);

		assert((MIN_PAGE - (pos + 1)) < MAX_PAGE);

		/* shift the old page to make the hole. */
		/* pos < MIN_PAGE so on copy size is >= 0 */
		copy_item_backward(&page->page_table[pos + 1],
				   &page->page_table[pos],
				   MIN_PAGE - (pos +  1));

		/* insert the item. */
		page->page_table[pos] = *value;
	} else if (pos > MIN_PAGE) {
		/* save the pivot. */
		*excess_elt = page->page_table[MIN_PAGE];

		assert((pos - (MIN_PAGE+1)) < MAX_PAGE);

		/* split-up the page. */
		copy_item(&new_page->page_table[0],
			  &page->page_table[MIN_PAGE + 1],
			  pos - (MIN_PAGE+1));

		/* insert the elt. */
		new_page->page_table[pos - (MIN_PAGE + 1)] = *value;

		assert(((int)(MAX_PAGE - pos)) >= 0);

		copy_item(&new_page->page_table[pos - MIN_PAGE],
			  &page->page_table[pos],
			  MAX_PAGE - pos);
	} else { /* pos  == MIN_PAGE */
		/* the pivot is the item to insert */
		*excess_elt = *value;

		/* split-up the page */
		copy_item(&new_page->page_table[0],
			  &page->page_table[MIN_PAGE],
			  MIN_PAGE);
	}

	/* can setup now the page */
	page->count = new_page->count = MIN_PAGE;
	new_page->p0 = excess_elt->child_page;
	excess_elt->child_page = new_page_idx;
}

static int do_reorg(root_t * root, page_idx_t page_idx, unsigned int pos,
		    value_t * excess_elt, value_t * value)
{
	int need_reorg;
	page_t * page;

	page = page_nr_to_page_ptr(root, page_idx);

	assert(page->count <= MAX_PAGE);
	
	/* the insertion pos can be at the end of the page so <= */
	assert(pos <= MAX_PAGE);

	if (page->count < MAX_PAGE) {
		/* insert at pos, shift to make a hole. */
		assert((page->count - pos < MAX_PAGE));

		copy_item_backward(&page->page_table[pos + 1],
				   &page->page_table[pos],
				   page->count - pos);

		page->page_table[pos] = *value;

		page->count++;

		need_reorg = 0;
	} else {
		page_idx_t new_page_idx = add_page(root);

		/* we can not pass page, the page pointer can be invalidated
		 * by add_page so pass page_idx here, call will re-get the page
		 * pointer */
		split_page(root, page_idx, new_page_idx, value,
			   excess_elt, pos);

		need_reorg = 1;
	}

	return need_reorg;
}

#if MIN_PAGE > 6
static int do_insert(root_t * root, page_idx_t page_idx,
		     value_t * excess_elt, value_t * value)
{
	int need_reorg;
	int left, right, pos;
	page_idx_t child_page_idx;
	page_t * page;

	page = page_nr_to_page_ptr(root, page_idx);

	assert(page->count != 0);

	left = 0;
	right = page->count - 1;

	/* a little what experimental, compiler with cond move insn
	 * can generate code w/o branching inside the loop */
	/* tests show than 60 to 70 % of cpu time comes from this loop. */
	do {
		pos = (left + right) >> 1;
		if (page->page_table[pos].key >= value->key)
			right = pos - 1;
		if (page->page_table[pos].key <= value->key)
			left = pos + 1;
	} while (left <= right);

	/* if left - right == 2 ==> found at pos.
	 * else if right == -1 that's a leftmost elt
	 * else left is insertion place */

	if (left - right == 2) {
		/* found: even if the write is non-atomic we do not need
		 * to lock() because we work only in the case of one writer,
		 * multiple reader. */
		page->page_table[pos].info += value->info;
		return 0;
	}

	if (right == -1) {
		child_page_idx = page->p0;
	} else {
		child_page_idx = page->page_table[right].child_page;
	}

	need_reorg = 0;

	if (child_page_idx != nil_page) {
		need_reorg = do_insert(root, child_page_idx,
				       excess_elt, value);
		*value = *excess_elt;
	} else {
		need_reorg = 1;
	}

	if (need_reorg) {
		need_reorg = do_reorg(root, page_idx, left, excess_elt, value);
	}

	return need_reorg;
}
#else
static int do_insert(root_t * root, page_idx_t page_idx,
		     value_t * excess_elt, value_t * value)
{
	int need_reorg;
	int pos;
	page_idx_t child_page_idx;
	page_t * page;

	page = page_nr_to_page_ptr(root, page_idx);

	assert(page->count != 0);

	for (pos = 0 ; pos < page->count ; ++pos) {
		if (page->page_table[pos].key >= value->key)
			break;
	}

	if (pos != page->count && page->page_table[pos].key == value->key) {
		/* found: even if the write is non-atomic we do not need
		 * to lock() because we work only in the case of one writer,
		 * multiple reader. */
		page->page_table[pos].info += value->info;
		return 0;
	}

	if (pos == 0) {
		child_page_idx = page->p0;
	} else {
		child_page_idx = page->page_table[pos-1].child_page;
	}

	need_reorg = 0;

	if (child_page_idx != nil_page) {
		need_reorg = do_insert(root, child_page_idx,
				       excess_elt, value);
		*value = *excess_elt;
	} else {
		need_reorg = 1;
	}

	if (need_reorg) {
		need_reorg = do_reorg(root, page_idx, pos, excess_elt, value);
	}

	return need_reorg;
}
#endif

void insert(root_t * root, unsigned int key, unsigned int info)
{
	value_t excess_elt;
	value_t value;
	int need_reorg;

	value.key = key;
	value.info = info;
	value.child_page = nil_page;

	if (!root->base_area) {
		/* create the root. */
		page_t * page;

		root->root = add_page(root);

		page = page_nr_to_page_ptr(root, root->root);

		page->page_table[0] = value;
		page->count = 1;
		return;
	}

	need_reorg = do_insert(root, root->root, &excess_elt, &value);
	if (need_reorg) {
		/* increase the level of tree. */
		page_t * new_page;
		page_idx_t old_root;

		old_root = root->root;
		root->root = add_page(root);

		/* page pointer can be invalidated by add_page, reload it */
		new_page = page_nr_to_page_ptr(root, root->root);

		new_page->page_table[0] = excess_elt;
		new_page->count = 1;
		new_page->p0 = old_root;
	}
}

