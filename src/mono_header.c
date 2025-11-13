#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/*
 * A @mono_block is defined in this way:
 * 
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
 * | mono_header |    Memory in use    |
 * +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-
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

typedef struct mono_block {
    size_t dimension;
    int free;
    struct mono_block *next;
}



