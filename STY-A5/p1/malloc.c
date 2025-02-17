#define USE_REAL_SBRK 1
#pragma GCC diagnostic ignored "-Wunused-function"

#if USE_REAL_SBRK
#define _GNU_SOURCE

#include <sys/mman.h>
#include <stddef.h>
#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <stdbool.h>
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
#include <stdbool.h>
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

// Starting with Best fit allocation, having it in a global variable so we can call setallocationstrategy and update it and it holds into the malloc function
static AllocType allocationStrategy = ALLOC_BESTFIT;

void setAllocationStrategy(AllocType type) {
    allocationStrategy = type;
}
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

Block *checkpoint = NULL;
 
void *my_malloc(uint64_t size)
{
    if (size == 0 || size > HEAP_SIZE - HEADER_SIZE) return NULL; // if size is 0 or bigger than the heap size, return NULL

	// Round up the size to the nearest multiple of 16
    bool moreMemory = false; // to allocate more memory
    uint64_t roundedSize = roundUp(size) + HEADER_SIZE; 
    Block *prev = NULL;  // previous block
    Block *current = _firstFreeBlock; // start at the first free block

    Block *fitFound = NULL; // best fit block
    Block *fitFoundPrev = NULL; // previous best fit block
    
    switch (allocationStrategy) {
        case ALLOC_BESTFIT: // Find the smallest block thats large enough
            while (current != NULL) {
                if (current->size >= roundedSize && (fitFound == NULL || current->size < fitFound->size)) { //if the current block is bigger than the size we need and the best fit is null or the current block is smaller than the best fit
                    fitFound = current;
                    fitFoundPrev = prev;
                }
                prev = current;
                current = current->next;
            }
            moreMemory = (!(fitFound == NULL));
            break;
        case ALLOC_FIRSTFIT: // Find the first block thats larger than what we need
            while (current != NULL) {
                if (current->size >= roundedSize) {
                    fitFound = current;
                    fitFoundPrev = prev;
                    break;
                }
                prev = current;
                current = current -> next;
            }
            moreMemory = (!(fitFound == NULL));
            break;
        case ALLOC_NEXTFIT:
            // We have two scenarios, either we have already called nextfit before and so we have a checkpoint to keep going from, or we start from the beginning
            if (checkpoint != NULL) {
                current = checkpoint;
            }
            
            while (current != NULL) {
                if (current->size >= roundedSize) {
                    fitFound = current;
                    fitFoundPrev = prev;
                    checkpoint = current->next;
                    break;
                }
                prev = current;
                current = current->next;
                // At some point we reach the end so next will become NULL and so current becomes NULL
                if (current == NULL) {
                    current = _firstFreeBlock; // so we start back at the beginning of the free list
                }
                // Stop searching when we reach the block where we started
                if (current == checkpoint) {
                    // Assuming that we got here because we needed more memory since size >= roundedSize did not trigger otherwise it would have broken out
                    // We only need to break here since moreMemory bool will be default false and so it will trigger allocating more memory
                    break;
                }
            }
            moreMemory = (!(fitFound == NULL));
            break;
        case ALLOC_WORSTFIT: // Find the largest block
            while (current != NULL) {
                if (current->size >= roundedSize && (fitFound == NULL || current->size > fitFound->size)) { 
                    fitFound = current;
                    fitFoundPrev = prev;
                }
                prev = current;
                current = current->next;
            }
            moreMemory = (!(fitFound == NULL));
            break;
    }

    if (!moreMemory) {
        uint64_t newHeapSize = _heapSize + HEAP_SIZE; // allocate more memory if we cant find a block that fits
        uint8_t *heapNew = allocHeap(_heapStart, newHeapSize);  // allocate the memory
        if (heapNew == NULL) {
            return NULL;
        }
        _heapSize = newHeapSize;
        Block *newBlock = (Block *)(_heapStart + _heapSize - HEAP_SIZE); // create a new block
        newBlock->size = HEAP_SIZE;
        newBlock->next = _firstFreeBlock;
        _firstFreeBlock = newBlock;
        return my_malloc(size);
    }

    Block **update_next = (fitFoundPrev ? &fitFoundPrev->next : &_firstFreeBlock); // update the next pointer
    return allocate_block(update_next, fitFound, roundedSize); // allocate the block
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
    if (address == NULL) { 
		return;
	}

    Block *block = (Block *)((uint8_t *)address - HEADER_SIZE); // get the block from the address
    if (block->next != (Block *)ALLOCATED_BLOCK_MAGIC) {
		return; // return if not allocated
	} 

    Block *prev = NULL;
    Block *current = _firstFreeBlock;

    while (current != NULL && current < block) { // find the right place to insert the block
        prev = current;
        current = current->next;
    }
 
    if (prev == NULL) { // if the block is the first free block
        block->next = _firstFreeBlock;
        _firstFreeBlock = block;
    } else { // insert the block
        block->next = current;
        prev->next = block;
    }
    // If free merged a block with its predecessor
    // 1:->[xxx] 2:->[xxx] , ->[xxxxxx], if we had a last allocation as the pointer to block nr 2,  we now then have to make it point to where block 1 started.

    if (block->next != NULL && (uint8_t *)block + block->size == (uint8_t *)block->next) { 
        if (checkpoint == block->next) {
            checkpoint = block;
        }
        merge_blocks(block, block->next);
        
    }

    if (prev != NULL && (uint8_t *)prev + prev->size == (uint8_t *)block) { 
        if (checkpoint == block) {
            checkpoint = prev;
        }
        merge_blocks(prev, block);
    }


}

MallocStat getAllocStatistics()
{
    MallocStat stat;
    stat.nFree = 0;
    stat.avgFree = 0;
    stat.largestFree = 0;
    
    Block *current = _firstFreeBlock;
    
    while (current != NULL) {
        stat.nFree++;
        stat.avgFree += current->size;
        if (current->size > stat.largestFree) {
            stat.largestFree = current->size;
            }
        current = current->next;    
        }
    stat.avgFree = stat.avgFree / stat.nFree;
    return stat;
}
