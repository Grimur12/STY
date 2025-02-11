#define USE_REAL_SBRK 0
#pragma GCC diagnostic ignored "-Wunused-function"

#if USE_REAL_SBRK
#define _GNU_SOURCE

#include <sys/mman.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <inttypes.h>

#include "malloc.h"

/* Function to allocate heap. Do not modify.
 * This is a wrapper around the system call sbrk.
 * For initialization, you can call this function as allocHeap(NULL, size)
 *   -> It will allocate a heap of size <size> bytes and return a pointer to the start address
 * For enlarging the heap, you can later call allocHeap(heapaddress, newsize)
 *   -> heapaddress must be the address previously returned by allocHeap(NULL, size)
 *   -> newsize is the new size
 *   -> function will return NULL (no more memory available) or heapaddress (if ok)
 */

uint8_t *allocHeap(uint8_t *currentHeap, uint64_t size)
{               
        static uint64_t heapSize = 0;
        if( currentHeap == NULL ) {
                uint8_t *newHeap  = sbrk(size);
                if(newHeap)
                        heapSize = size;
                return newHeap;
        }
	uint8_t *newstart = sbrk(size - heapSize);
	if(newstart == NULL) return NULL;
	heapSize += size;
	return currentHeap;
}
#else
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdlib.h>
#include <inttypes.h>

#include "malloc.h"
// This is a "fake" version that you can use on MacOS
// sbrk as used above is not available on MacOS
// and normal malloc allocation does not easily allow resizing the allocated memory
uint8_t *allocHeap(uint8_t *currentHeap, uint64_t size)
{
        static uint64_t heapSize = 0;
        if( currentHeap == NULL ) {
                uint8_t *newHeap  = malloc(10*size);
                if(newHeap)
                        heapSize = 10*size;
                return newHeap;
        }
	if(size <= heapSize) return currentHeap;
        return NULL;
}
#endif


/*
 * This is the heap you should use.
 * (Initialized with a heap of size HEAP_SIZE) in initAllocator())
 */
uint8_t *_heapStart = NULL;
uint64_t _heapSize = 0;

/*
 * This should point to the first free block in memory.
 */
Block *_firstFreeBlock;

/*
 * Initializes the memory block. You don't need to change this.
 */
void initAllocator()
{	
    _heapStart = allocHeap(NULL, HEAP_SIZE);
	_heapSize = HEAP_SIZE;

	/* Add more initialization below, e.g. for the free block list */
	// See lab tutorial
	_firstFreeBlock = (Block *)_heapStart;
    _firstFreeBlock->size = HEAP_SIZE;
    _firstFreeBlock->next = NULL;

}


/*
 * Gets the next block that should start after the current one.
 */
static Block *_getNextBlockBySize(const Block *current)
{
	Block *end = (Block*)((uint8_t *)_heapStart + _heapSize);
	Block *next = (Block *)((uint8_t *)current + current->size);

	assert(next <= end);
	return (next == end) ? NULL : next;
	}
	// Works as well:
	// Block *next = (Block *)( (uint8 t *)current + current−>size );
	//
	// Note that in this case the cast to (uint8 t *) is essential.
	// The type of current is (Block *). Without the cast,
	// current + current−>size would mean adding sizeof(Block)*current−>size

/*
 * Dumps the allocator. You should not need to modify this.
 */
void dumpAllocator()
{
	Block *current;
	/* Note: This sample code prints addresses relative to the beginning of the heap */
	/* Part a: all blocks, using size, and starting at the beginning of the heap */

	current = (Block*)_heapStart;
	printf("All blocks:\n");
	while (current) 
	{
		printf("Block starting at %" PRIuPTR ", size %" PRIu64 "(%s)\n",
		((uintptr_t)(void*)current - (uintptr_t)(void*)_heapStart),
		current->size,
		current->next == ALLOCATED_BLOCK_MAGIC ? "in use" : "free" );
		
		current = _getNextBlockBySize(current);
	}
	/* Part b: free blocks, using next pointer, and starting at firstFreeBlock */
	printf ("Free block list:\n");
	current = _firstFreeBlock;
	while (current) 
	{
		printf (" Free block starting at %" PRIuPTR ", size %" PRIu64 "\n",
		((uintptr_t)(void*)current - (uintptr_t)(void*)_heapStart),
		current->size);
		
		current = current->next;
	}
}

/*
 * Round the integer up to the block header size (16 Bytes).
 */
uint64_t roundUp(uint64_t n)
{
	return (n+0xF) & (~0xF);
}

/* Helper function that allocates a block 
 * takes as first argument the address of the next pointer that needs to be updated (!)
 */
static void *allocate_block(Block **update_next, Block *block, uint64_t new_size)
{
	if (block->size == new_size) 
	{
		*update_next = block->next; // take it from the free list
	} else if (block->size > new_size) 
	{
		Block *extra = (Block *)((uint8_t *)block + new_size); // To get the space we didnt need to allocate since its bigger
		extra->size = block->size - new_size; // extra space
		extra->next = block->next;
		*update_next = extra; // completely taking it out of the free list
		block->size = new_size; // then we can let it allocate with the required size
	}
	block->next = (Block *)ALLOCATED_BLOCK_MAGIC;
	return block->data; // returning pointer to the beginning of the blocks data area
}

void *my_malloc(uint64_t size)
{
    if (size == 0) return NULL;

    uint64_t roundedSize = roundUp(size) + HEADER_SIZE;
    Block *prev = NULL;
    Block *current = _firstFreeBlock;
    Block *bestFit = NULL;
    Block *bestFitPrev = NULL;

    while (current != NULL) {
        if (current->size >= roundedSize) {
            if (bestFit == NULL || current->size < bestFit->size) {
                bestFit = current;
                bestFitPrev = prev;
            }
        }
        prev = current;
        current = current->next;
    }

    if (bestFit == NULL) {
        uint64_t newHeapSize = _heapSize + HEAP_SIZE;
        uint8_t *newHeap = allocHeap(_heapStart, newHeapSize);
        if (newHeap == NULL) return NULL;
        _heapSize = newHeapSize;
        Block *newBlock = (Block *)(_heapStart + (_heapSize - HEAP_SIZE));
        newBlock->size = HEAP_SIZE;
        newBlock->next = _firstFreeBlock;
        _firstFreeBlock = newBlock;
        return my_malloc(size);
    }

    return allocate_block((bestFitPrev ? &bestFitPrev->next : &_firstFreeBlock), bestFit, roundedSize);
}



/* Helper function to merge two freelist blocks.
 * Assume: block1 is at a lower address than block2
 * Does nothing if blocks are not neighbors (i.e. if block1 address + block1 size is not block2 address)
 * Otherwise, merges block by merging block2 into block1 (updates block1's size and next pointer
 */
static void merge_blocks(Block *block1, Block *block2)
{
	// Simply just merge the sizes, so make block 1 absorb block 2 and then skip over block 2 in the list.
	block1->size += block2->size;
	block1->next = block2->next;
}


void my_free(void *address)
{
    if (address == NULL) return;

    Block *block = (Block *)((uint8_t *)address - HEADER_SIZE);
    if (block->next != (Block *)ALLOCATED_BLOCK_MAGIC) {
		
	} return; // Ensure it was allocated

    Block *prev = NULL;
    Block *current = _firstFreeBlock;

    // Find the correct insertion point (sorted order)
    while (current != NULL && current < block) {
        prev = current;
        current = current->next;
    }

    // Insert block into the free list
    if (prev == NULL) {
        block->next = _firstFreeBlock;
        _firstFreeBlock = block;
    } else {
        block->next = current;
        prev->next = block;
    }

    // Try merging with the next block
    if (block->next != NULL && (uint8_t *)block + block->size == (uint8_t *)block->next) {
        merge_blocks(block, block->next);
    }

    // Try merging with the previous block
    if (prev != NULL && (uint8_t *)prev + prev->size == (uint8_t *)block) {
        merge_blocks(prev, block);
    }
}