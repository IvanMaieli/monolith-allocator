#define _GNU_SOURCE
#define DIM_ARENA 4096

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/mman.h>

/*
 * A @mono_block is defined in this way:
 * 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
 * | mono_header |        Memory in use       |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
 * 
 * In this way every block of the @arena, i.e.
 * the memory available to the process, is splitted
 * in blocks, that the programmer can request if needed.
 * Every block has a @mono_head prefixed before the 
 * actual memory, that allows the allocator to know
 * what block is free, what is his dimension and what is 
 * the next block in memory to check.
 *
 * 
 * @mono_head is a monolith-alloc structure 
 * that is used as a free list. Basically it allows to
 * link the available pages of memory, storing
 * info about freedom, dimension of the blocks and next
 * element of the list.
 *
 * @dimension: number of bytes of the block
 * @free: boolean fields that holds if the block is used
 * @next: the next block of memory in the free list
 */

typedef struct mono_header {
    size_t dim;
    int8_t free;
    struct mono_header *next;
} mono_header;

// Starting pointer of the arena (full page allocated for the process)
void *arena = NULL;

/*
 * Request di @arena, i.e. all the space allocated for
 * the specific process, then creates the @mh (mono_header)
 * pointing at the beginning of the @arena, then sets
 * the fields of the header as a whole one single free page
 * of memory, prefixed by an header. 
 */
void mono_init() {
    arena = mmap(NULL, DIM_ARENA, 
                       PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS,
                       -1, 0);
    mono_header *mh = (mono_header *) arena;
    mh->dim = DIM_ARENA - sizeof(mono_header); 
    mh->free = 1;
    mh->next = NULL;
}


/*
 * This function returns a pointer to the allocated memory requested.
 * The new header is set with offset 'new_header + requested_mem + 1_byte' 
 * (idc about casting in this docs, imagine that everything is in bytes, not
 * chunks of mem of structs), so this creates the allocation and translates 
 * the free memory ahead. 
 */
void *mono_loc(size_t usize) {
    mono_header *mh = (mono_header *) arena;
    if (mh->dim < (usize + sizeof(mono_header)))         
        return NULL;

    mono_header *next_h = (mono_header *) ((int8_t *) mh + sizeof(mono_header) + usize);
    next_h->dim = mh->dim - sizeof(mono_header) - usize;
    next_h->free = 1;
    next_h->next = NULL;

    mh->dim = usize;
    mh->free = 0;
    mh->next = next_h;

    return (void *) (mh + 1);
}

int main(void){ return 0; }






