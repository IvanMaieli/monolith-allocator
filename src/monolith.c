#define GNU_SOURCE
#define REQUESTED_MEMORY_BYTES 4096
#define MIN_BLOCK_SIZE 32
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>

void *STARTING_ADDRESS = NULL;


typedef struct TH_block_t {
    size_t size;
    int8_t free;
    struct TH_block_t *next;
    struct TH_block_t *prev;
} TH_block_t;

void monolith_init() {
    STARTING_ADDRESS = mmap(NULL, REQUESTED_MEMORY_BYTES,
                        PROT_READ | PROT_WRITE,
                        MAP_PRIVATE | MAP_ANONYMOUS,
                        -1, 0);
    if (STARTING_ADDRESS == MAP_FAILED) {
        perror("mmap could not allocate the memory");
        exit(EXIT_FAILURE);
    }

    TH_block_t *block = (TH_block_t *) STARTING_ADDRESS;
    block->size = REQUESTED_MEMORY_BYTES - sizeof(TH_block_t);
    block->free = 1;
    block->next = NULL;
    block->prev = NULL;
}

static TH_block_t *TH_find_block(const size_t size) {
    TH_block_t *current = (TH_block_t *) STARTING_ADDRESS;
    while (current != NULL && (current->size < size || !current->free)) {
        current = current->next;
    }
    return current;
}

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

void *TH_malloc(const size_t size) {
    TH_block_t *block = TH_find_block(size);
    if (!block) return NULL;
    if (block->size >= size + sizeof(TH_block_t) + MIN_BLOCK_SIZE)
        TH_split_block(block, size);
    else block->free = 0;

    return (void *) (block + 1);
}

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