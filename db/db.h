
/** a tiny data base struct designed for oprofile
 * allow only an insert operation
 */

#ifndef DB_H
#define DB_H

#include <assert.h>

/* must be in [2-a sensible value] */
/* in function of MIN_PAGE two different search algo are selected at compile
 * time, a straight liner search or a dicho search, the best perf for the
 * two method are equal at approximately 6 and 64 item. I defer to later
 * the page size choice. See do_insert() */
#define MIN_PAGE 6
#define MAX_PAGE MIN_PAGE*2

typedef unsigned int page_idx_t;

#define nil_page	(page_idx_t)~0

/** an item */
typedef struct {
	page_idx_t child_page;		/*< right page index */
	unsigned int info;		/*< sample count in oprofile */
	unsigned int key;		/*< eip in oprofile */
} value_t;

/** a page of item */
typedef struct {
	unsigned int count;		/*< nr entry used in page_table */
	page_idx_t p0;			/*< left page index */
	value_t page_table[MAX_PAGE];	/*< key, data and child pointer */
} page_t;

/** a "database", for oprofile this a part of the samples files header */
typedef struct {
	page_t * base_area;		/*< base memory area */
	page_idx_t root;		/*< the root page index */
	unsigned int size;		/*< nr page */
	unsigned int current_size;	/*< nr used page */
	// TODO we need a lock ? or do you want to lock the file itself ?
	// perhaps a semaphore is more efficient ?
} root_t;

/* db-manage.c */
void init_root(root_t * root);
void delete_root(root_t * root);

/** add a page returning his index. Take care all page pointer are
 * invalidated by this call ! */
page_idx_t add_page(root_t * root);

int check_tree(const root_t * root, page_idx_t page_nr, unsigned int last);
void display_tree(const root_t * root, page_idx_t page_nr);

/* db-insert.c */
/** insert info at key, if key already exist the info is added to the
 * existing samples */
void insert(root_t * root, unsigned int key, unsigned int info);

/** from a page index return a page pointer */
static __inline page_t * page_nr_to_page_ptr(const root_t * root,
					     page_idx_t page_nr)
{
	assert(page_nr < root->current_size);
	return &root->base_area[page_nr];
}

#endif /* !DB_H */
