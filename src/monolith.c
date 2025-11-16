#define GNU_SOURCE
#define REQUESTED_MEMORY_BYTES 4096
#define MIN_BLOCK_SIZE 8

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

void *STARTING_ADDRESS = NULL;

/* @TH_block_t
* This struct is the foundational point of the API.
* This is a single node in a double-linked list.
* A chunk, or block, of requested memory is organized like this:
*
* +++++++++++++++++++++++++++++++++++++++
* | TH_block_t |       Avail. Mem       |
* +++++++++++++++++++++++++++++++++++++++
*
* Every requested memory has its own header that keeps the data
* of the block:
* @size is the size of the available memory in bytes
* @free keeps if the block is being used or not;
* @next is the next node of the list, it may be 'NULL' if it's the last;
* @prev is the previous node of the list,
* it may be 'NULL' if it is the first;
*/
typedef struct TH_block_t {
    size_t size;
    int8_t free;
    struct TH_block_t *next;
    struct TH_block_t *prev;
} TH_block_t;


/* @TH_init()
* This procedure is used to request the first memory with the mmap() call.
* Then It sets the first header for the memory. (@STARTING_ADDRESS).
*/
void TH_init() {
    STARTING_ADDRESS = mmap(NULL, REQUESTED_MEMORY_BYTES,
                        PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS,
                        -1, 0);
    if (STARTING_ADDRESS == MAP_FAILED) {
        perror("mmap() call could not allocate memory");
        exit(EXIT_FAILURE);
    }

    TH_block_t *block = (TH_block_t *) STARTING_ADDRESS;
    block->size = REQUESTED_MEMORY_BYTES - sizeof(TH_block_t);
    block->free = 1;
    block->next = NULL;
    block->prev = NULL;
}

/* @TH_find_block()
* This function searches a block that is big enough
* to contain another allocation of @size bytes.
*/
static TH_block_t *TH_find_block(const size_t size) {
    TH_block_t *current = (TH_block_t *) STARTING_ADDRESS;
    while (current != NULL && (current->size < size || !current->free)) {
        current = current->next;
    }
    return current;
}

/* @TH_split_block()
* This function split a block in two different blocks.
* If we find a slice of memory big enough for the alloc,
* it may be too big, so we can waste a lot of memory.
* So we check if we can break the block in two.
* The minimum size, set in @MIN_BLOCK_SIZE constant, is
* the minimum size for each block to be split.
*/
static void TH_split_block(TH_block_t *block, const size_t size) {
    TH_block_t *new_block = (TH_block_t*) ( (char *) block + sizeof(TH_block_t) + size );
    new_block->size = block->size - size - sizeof(TH_block_t);
    new_block->free = 1;
    new_block->next = block->next;
    new_block->prev = block;

    block->size = size;
    block->free = 0;
    block->next = new_block;

    if (new_block->next != NULL) {
        new_block->next->prev = new_block;
    }
}

/* @TH_malloc()
* This function allocates memory.
* It uses @TH_split_block() and @TH_find_block()
* in order to find where to allocate memory.
*/
void *TH_malloc(const size_t size) {
    TH_block_t *block = TH_find_block(size);
    if (!block) return NULL;
    if (block->size >= size + sizeof(TH_block_t) + MIN_BLOCK_SIZE)
        TH_split_block(block, size);
    else block->free = 0;

    return (void *) (block + 1);
}

/* @TH_calloc()
* This function allocates memory setting an init value.
* It uses @TH_malloc() to allocate memory, then @memset()
* to assign values.
*/
void *TH_calloc(const int num, const size_t size) {
    const size_t requested = num * size;
    if (requested <= 0) return NULL;
    void *ptr = TH_malloc(requested);
    if (ptr) memset(ptr, 0, requested);
    return ptr;
}

/* @TH_free()
* This function frees the memory.
* It merges close blocks together if they are mergeable (left and right).
* On the left side I just shifted the actual location to free.
* This allowed me to have the mergeable blocks only on the right...
* On the right I merged blocks.
*/
void TH_free(const void *ptr) {
    if (ptr == NULL) return;
    TH_block_t *block = (TH_block_t *) ptr - 1;
    block->free = 1;

    TH_block_t *prev = block->prev;
    while (prev != NULL && prev->free) {
        block = prev;
        prev = block->prev;
    }

    TH_block_t *next = block->next;
    while (next != NULL && next->free) {
        block->size += sizeof(TH_block_t) + next->size;
        next = next->next;
    }
    block->next = next;
    if (next != NULL) next->prev = block;
}


int main(int argc, char *argv[]) {

    return 0;
}
