# Monolith

A custom dynamic memory allocator implementation written in C.

Monolith is a **drop-in replacement** for the standard library's memory allocation functions (`malloc`, `free`, and `calloc`). This project serves as a deep dive into low-level systems programming, demonstrating robust heap management, precise pointer arithmetic, and effective strategies to combat memory fragmentation.

It is designed to be an educational but fully functional piece of infrastructure, capable of being injected into production-level C programs.

---

## 🚀 Key Features

* **Doubly-Linked List Architecture:** The entire memory arena is structured as a continuous chain of blocks (both free and allocated) linked via `next` and `prev` pointers. This foundation is crucial for efficient bidirectional traversal and merging.
* **Memory Acquisition (`mmap`):** The allocator utilizes the `mmap` system call to request large, contiguous memory regions (the Arena) directly from the kernel, avoiding the limitations and overhead of older methods like `sbrk`.
* **First-Fit Search Strategy:** The `malloc` implementation scans the list for the first block that is sufficiently large to satisfy the user's request.
* **Aggressive Bidirectional Coalescing:** The `free` function employs a continuous merging strategy (using `while` loops for both left and right directions) to unite all adjacent free blocks into a single large block, thus drastically reducing external fragmentation.
* **Block Splitting:** When a free block is larger than needed, it is surgically split (`split_block`) into two: one piece for the user and the remainder returned to the free list, minimizing internal fragmentation.
* **Thread Safety (Base):** Access to the heap is protected by a global `pthread_mutex`, making the allocator safe for use in multi-threaded applications (though performance under high contention is secondary to correctness).

---

## 🧱 Core Data Structure

The entire memory management logic is built around this single structure, which serves as the metadata header for every block in the arena:

```c
typedef struct TH_block_t {
    /** Size of the payload (user space) ONLY, not including the header itself. */
    size_t size;
    
    /** Flag: 1 if the block is free, 0 if allocated. */
    int8_t free;
    
    /** Pointer to the PHYSICALLY next BlockHeader in the heap chain. */
    struct TH_block_t *next;
    
    /** Pointer to the PHYSICALLY previous BlockHeader in the heap chain. */
    struct TH_block_t *prev;
} TH_block_t;
