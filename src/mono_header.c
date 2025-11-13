#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


/* @mono_block is a monolith-alloc structure 
 * that is used as a free list. Basically it allows to
 * link the available pages of memory, storing
 * info about freedom, dimension of the blocks and next
 * element of the list 
 */

typedef struct mono_block {
    size_t dimension;
    int free;
    struct mono_block *next;
}


