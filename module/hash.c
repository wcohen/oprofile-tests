#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define u16 unsigned short
#define u32 unsigned int
#define uint unsigned int

/** 65536 * 32 = 2097152 bytes default */
#define OP_DEFAULT_HASH_SIZE 65536
/** nr entry by hash table entry */
#define OP_NR_ENTRY 4

/* the best known solution */
static unsigned int RSHIFT_EIP_1 = 2;
static unsigned int LSHIFT_PID = 0;
static unsigned int LSHIFT_EIP_2 = 10;
static unsigned int LSHIFT_CTR = 8;
/* 0 0 8 0 is also a good solution and produce better code for hash coding
 * calculus */

static __inline unsigned int op_hash(u32 eip, u16 pid, int ctr)
{
	return 	((eip >> RSHIFT_EIP_1) ^ 
		 (pid << LSHIFT_PID) ^ 
		 (eip << LSHIFT_EIP_2) ^ 
		 (ctr << LSHIFT_CTR)) & (OP_DEFAULT_HASH_SIZE - 1);

}

/** maximum number of counters, up to 4 for Athlon (18 for P4). The primary
 * use of this variable is for static/local array dimension. Never use it in 
 * loop or in array index access/index checking unless you know what you
 * made. Don't change it without updating OP_BITS_CTR! */
#define OP_MAX_COUNTERS	4

/** the number of bits neccessary to store OP_MAX_COUNTERS values */
#define OP_BITS	2

/** The number of bits available to store count. The 16 value is
 * sizeof_in_bits(op_sample.count)  */
#define OP_BITS_COUNT	(16 - OP_BITS)

/** counter nr mask */
#define OP_CTR_MASK	((~0U << (OP_BITS_COUNT + 1)) >> 1)

/** top OP_BITS bits of count are used to store counter number */
#define OP_COUNTER(x)	(((x) & OP_CTR_MASK) >> OP_BITS_COUNT)
/** low bits store the counter value */
#define OP_COUNT_MASK	((1U << OP_BITS_COUNT) - 1U)

/** Data type to transfer samples counts from the module to the daemon */
struct op_sample {
	/** samples count; high order bits contains the counter nr */
	u16 count;
	u16 pid;	/**< 32 bits but only 16 bits are used currently */
	u32 eip;	/**< eip value where occur interrupt */
} __attribute__((__packed__, __aligned__(8)));

struct op_entry {
	struct op_sample samples[OP_NR_ENTRY];
};

/* per-cpu dynamic data */
struct _oprof_data {
	/* hash table */
	struct op_entry *entries; 
	/* next sample in entry */
	uint next; 
	/* reset counter values */
	uint ctr_count[OP_MAX_COUNTERS]; 
};

/* is the count at maximal value ? */
#define op_full_count(c) (((c) & OP_COUNT_MASK) == OP_COUNT_MASK)

/* no check for ctr needed as one of the three will differ in the hash */
#define op_miss(ops)  ((ops).eip != eip || \
	(ops).pid != pid || \
	op_full_count((ops).count))

static struct _oprof_data oprof_data;
static unsigned int nr_samples;
static unsigned int nr_eviction = 0;
static unsigned int nr_min_eviction = (unsigned int)-1;

static __inline void evict_op_entry(uint cpu, struct _oprof_data * data,
			   const struct op_sample *ops)
{
	nr_eviction++;
}

static __inline void fill_op_entry(struct op_sample *ops, u16 pid, u32 eip, int ctr)
{
	ops->eip = eip;
	ops->pid = pid;
	ops->count = (1U << OP_BITS_COUNT)*ctr + 1;
}

void op_do_profile(uint cpu, u16 pid, u32 eip, int ctr)
{
	struct _oprof_data * data = &oprof_data;

	uint h = op_hash(eip, pid, ctr);
	uint i;

	nr_samples++;

	for (i=0; i < OP_NR_ENTRY; i++) {
		if (!op_miss(data->entries[h].samples[i])) {
			data->entries[h].samples[i].count++;
			return;
		} else if (op_full_count(data->entries[h].samples[i].count)) {
			goto full_entry;
		} else if (!data->entries[h].samples[i].count)
			goto new_entry;
	}

	evict_op_entry(cpu, data, &data->entries[h].samples[data->next]);
	fill_op_entry(&data->entries[h].samples[data->next], pid, eip, ctr);
	data->next = (data->next + 1) % OP_NR_ENTRY;
	return;
full_entry:
	evict_op_entry(cpu, data, &data->entries[h].samples[i]);
new_entry:
	fill_op_entry(&data->entries[h].samples[i], pid, eip, ctr);
	return;
}

void display_hash_state(void)
{
	int i, j;
	int nr_used = 0;

	for (i = 0 ; i < OP_DEFAULT_HASH_SIZE ; ++i) {
		for (j = 0 ; j < OP_NR_ENTRY ; ++j) {
			if (oprof_data.entries[i].samples[j].count & OP_COUNT_MASK)
				nr_used++;
		}
	}

	printf("nr_entry %d, nr_samples %d, nr_used entry %d, nr_eviction %d\n",
	       OP_DEFAULT_HASH_SIZE*OP_NR_ENTRY, nr_samples,
	       nr_used, nr_eviction);
}

void do_test(const char * filename)
{
	struct op_sample samples[128];
	int count;
	FILE * fp;
	unsigned int hash_size;

	hash_size = sizeof(struct op_entry) * OP_DEFAULT_HASH_SIZE;

	oprof_data.entries = malloc(hash_size);

	memset(oprof_data.entries, 0, hash_size);

	nr_samples = nr_eviction = 0;

	fp = fopen(filename, "r");

	while ((count = fread(samples, sizeof(struct op_sample), 128, fp)) != 0) {
		int i;
		for (i = 0 ; i < count ; ++i) {
			op_do_profile(0, samples[i].pid, samples[i].eip,
				      OP_COUNTER(samples[i].count));
		}
	}

/*	display_hash_state();*/

	if (nr_eviction <= nr_min_eviction) {
		nr_min_eviction = nr_eviction;
		printf("candidate: nr_eviction %d params %d %d %d %d\n",
		       nr_eviction, RSHIFT_EIP_1, LSHIFT_PID, LSHIFT_EIP_2,
		       LSHIFT_CTR);
	}

	free(oprof_data.entries);

	fclose(fp);
}

#define FOR_x(x, y) for (x = 0 ; x < y ; ++x)

int main(int argc, char ** argv)
{
	int i = 0;
	FOR_x(RSHIFT_EIP_1, 16) {
		FOR_x(LSHIFT_PID, 16) {
			FOR_x(LSHIFT_EIP_2, 16) {
				FOR_x(LSHIFT_CTR, 1) {
					fprintf(stderr, "test %d on %d\r",
						++i, 16*16*16);
					do_test("samples.dat");
				}
			}
		}
	}
	return 0;
}
