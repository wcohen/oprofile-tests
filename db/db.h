/**
 * \file db.h
 * Copyright 2002 OProfile authors
 * Read the file COPYING
 * this file contains various definitions and iterface for management
 * of in-memory Btree.
 *
 * \author Philippe Elie <phil.el@wanadoo.fr>
 */

#ifndef DB_H
#define DB_H

#include <assert.h>

#ifndef fd_t
#define fd_t int
#endif

/* must be in [2-a sensible value] */
/* in function of MIN_PAGE two different search algo are selected at compile
 * time, a straight liner search or a dicho search, the best perf for the
 * two method are equal at approximately 6 and 64 item. I defer to later
 * the page size choice. See do_insert(). Test show than the better performance
 * depend on the distribution of key, e.g. for 1E6 item distributed through
 * 1E4 distinct key 64 give better performance but for 1E5 distinct key 6
 * give better performance */
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
	value_t page_table[MAX_PAGE];	/*< key, data and child index */
} page_t;

/** the minimal information which must be stored in the file to reload
 * properly the data base */
typedef struct {
	unsigned int size;		/*< in page nr */
	unsigned int current_size;	/*< nr used page */
	page_idx_t root_idx;		/*< the root page index */
} db_descr;

/** a "database". this is an in memory only description.
 *
 * The mmaped file must contain a db_descr struct which manage the current
 * state of memory allocation (base pointer, root_idx etc.) the point here
 * is to allow using this library w/o knowning the offset inside the mapped
 * file of management data or the page base memory offset. A typical is use is:
 *
 * struct header { int etc; ... struct db_descr descr; .... };
 * db_open(&root, filename, offsetof(header, descr), sizeof(header));
 * so on this library have no dependency on the header type.
 */
typedef struct {
	page_t * page_base;		/*< base memory area of the page */
	fd_t fd;			/*< file descriptor of the maped mem */
	void * base_memory;		/*< base memory of the maped memory */
	db_descr * descr;		/*< the current state of database */
	unsigned int offset_descr;	/*< from base_memory to descr */
	unsigned int offset_page;	/*< from base_memory to page_base */
	unsigned int is_locked;		/*< is fd already locked */
} root_t;

/* db-manage.c */
/** 
 * \param root the data base object to setup 
 * \param root_idx_ptr an external pointer to put the root index, can be null
 * \param filename the filename where go the maped memory
 * \param offset_page offset between the mapped memory and the data base page
 * area.
 *
 * parameter root_idx_ptr and offset allow to use a data base imbeded in
 * a file containing an header such as opd_header. db_open always preallocate
 * a few number of page
 */
void db_open(root_t * root, const char * filename, unsigned int offset_page,
	     unsigned int offset_descr);
/**
 * \param root the data base to close
 */
void db_close(root_t * root);

/** add a page returning its index. Take care all page pointer can be
 * invalidated by this call ! */
page_idx_t add_page(root_t * root);

/** db-debug.c */
/* check than the tree is well build by making a check_page_pointer() then
 * checking than item are correctly sorted */
int check_tree(const root_t * root);
/* check than child page nr are coherent */
int check_page_pointer(const root_t * root);
/* display the item in tree */
void display_tree(const root_t * root);
/* same as above but do not travel through the tree, just display raw page */
void raw_display_tree(const root_t * root);

/* db-insert.c */
/** insert info at key, if key already exist the info is added to the
 * existing samples */
void insert(root_t * root, unsigned int key, unsigned int info);

/* db-travel.c */
/** the call back type to pass to travel() */
typedef void (*travel_callback)(unsigned int key, unsigned int info,
				void * data);
/* iterate through key in rang [first, last[ passing it to callback,
 * data is an optionnal user data to pass to the callback */
void travel(const root_t * root, unsigned int first, unsigned int last,
	    travel_callback callback, void * data);

/** from a page index return a page pointer */
static __inline page_t * page_nr_to_page_ptr(const root_t * root,
					     page_idx_t page_nr)
{
	assert(page_nr < root->current_size);
	return &root->page_base[page_nr];
}

#endif /* !DB_H */
