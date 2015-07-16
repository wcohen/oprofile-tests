/*
 *  This test case is to make sure that oprofile collects the samples
 *  hit at the extended region of an anon buffer.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>

typedef int (*func_t)(void);

/* If MINIMUM_BUFFER_SIZE is too small (like 4 KB), the anon buffer can
   overlap with the memory region of other library files such as ld-...,
   so any samples hit at the anon buffer are incorrectly attributed to
   the library files.  10 MB is large enough on Ubuntu 14.04/x86. */
#define MINIMUM_BUFFER_SIZE (10 * 1024 * 1024)

#if defined (__i386__) || defined (__x86_64__)
/* add $0x1,%eax */
char add_inst[3]={0x83, 0xc0, 0x01};
/* retq */
char ret_inst[1]={0xc3};
#elif defined(__arm__)
/* add r0, r0, #1 */
char add_inst[4]={0x01, 0x00, 0x80, 0xe2};
/* bx lr */
char ret_inst[4]={0x1e, 0xff, 0x2f, 0xe1};
#elif defined(__aarch64__)
/* add x0, x0, #1 */
char add_inst[4]={0x00, 0x04, 0x00, 0x91};
/* ret */
char ret_inst[4]={0xc0, 0x03, 0x5f, 0xd6};
#elif defined(__PPC__)
#if defined(__LITTLE_ENDIAN__)
/* addi r3,r3,1 */
char add_inst[4]={0x01, 0x00, 0x63, 0x38};
/* blr */
char ret_inst[4]={0x20, 0x00, 0x80, 0x4e};
#else
/* addi r3,r3,1 */
char add_inst[4]={0x38, 0x63, 0x00, 0x01};
/* blr */
char ret_inst[4]={0x4e, 0x80, 0x00, 0x20};
#endif
#else
  error unsupported architecture
#endif

/* Fill the extended region of the buffer with add and ret.

buffer                    func_start_addr
|                         |
v                         v
<-- initial_buffer_size -->
<-----            remapped_buffer_size                ----->
|.........................|add add add add add ... add ret|
 */
static func_t
initialize_anon_func(char *buffer, size_t initial_buffer_size, size_t remapped_buffer_size)
{
  char *func_start_addr = buffer + initial_buffer_size;
  char *func_end_addr = buffer + remapped_buffer_size;
  char *cursor;
  int add_size = sizeof(add_inst);
  int ret_size = sizeof(ret_inst);

#if defined(__powerpc64__) && defined(__BIG_ENDIAN__)
  {
    /* Have function descriptors rather than regular function pointers.
       https://refspecs.linuxfoundation.org/ELF/ppc64/PPC-elf64abi-1.9.html */
    char **fdesc  = (char **) func_start_addr;
    fdesc[0] = func_start_addr + sizeof (char *) * 3; /* pointer to code */
    fdesc[1] = 0; /* GOT */
    fdesc[2] = 0; /* evironment pointer */
    cursor = fdesc[0];
  }
#else
  cursor = func_start_addr;
#endif

  while (cursor + add_size + ret_size <= func_end_addr) {
    memcpy(cursor, add_inst, add_size);
    cursor += add_size;
  }
  memcpy(cursor, ret_inst, ret_size);

  return (func_t)func_start_addr;
}

int
main(int argc, char *argv[])
{
  void *buffer;
  int ret;
  size_t page_size;
  size_t initial_buffer_size;
  size_t remapped_buffer_size;
  func_t anon_func;
  int iter;
  int iter_max;

  if (argc == 2)
    iter_max = atoi(argv[1]);
  else
    iter_max = 2000;

  page_size = (size_t)sysconf(_SC_PAGESIZE);
  initial_buffer_size = (MINIMUM_BUFFER_SIZE / page_size + 1) * page_size;
  /* remapped_buffer_size must be larger than initial_buffer_size */
  remapped_buffer_size = initial_buffer_size * 2;

  /* First, reserve remapped_buffer_size bytes of virtual memory.
     Don't specify PROT_EXEC. This mmap is to increase the chance of
     the following two mmap's to succeed. */
  buffer = mmap(NULL, remapped_buffer_size, PROT_NONE, MAP_PRIVATE | MAP_ANON | MAP_NORESERVE, 0, 0);
  if (buffer == MAP_FAILED) {
    perror("mmap");
    exit(1);
  }

  /* This munmap might be unnecessary. */
  ret = munmap(buffer, remapped_buffer_size);
  if (ret == -1) {
    perror("munmap");
    exit(1);
  }

  /* Next, mmap initial_buffer_size bytes from the "buffer" address
     with PROT_EXEC. The kernel will notify oprofile of the new
     executable mmap region.
  */
  buffer = mmap(buffer, initial_buffer_size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_FIXED, 0, 0);
  if (buffer == MAP_FAILED) {
    perror("second mmap");
    exit(1);
  }

  /* Lastly, extend the buffer to remapped_buffer_size bytes with PROT_EXEC.
     The kernel will notify oprofile again.
     mremap(2) should be used instead? */
  buffer = mmap(buffer, remapped_buffer_size, PROT_EXEC | PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANON | MAP_FIXED, 0, 0);
  if (buffer == MAP_FAILED) {
    perror("third mmap");
    exit(1);
  }

  anon_func = initialize_anon_func(buffer, initial_buffer_size, remapped_buffer_size);

  /* Execute the function generated in the extended region of the buffer
     many times so that a number of samples hit there. */
  for (iter = 0; iter < iter_max; iter++) {
    anon_func();
  }
  exit(EXIT_SUCCESS);
}
