#pragma GCC diagnostic ignored "-Wunused-function"

#define _POSIX_C_SOURCE 200112L
#include "page_table.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

// The pointer to the base directory.
// You can safely assume that this is set before any address conversion is done.
static PageDirectory *_cr3 = NULL;

void setPageDirectory(PageDirectory *directory)
{
    _cr3 = directory;
}

#define ENTRY_MASK (ENTRIES_PER_TABLE - 1)

// Returns the base address of the current frame
// (i.e., the address of the first byte in the frame)
static inline uint32_t _getVirtualBase(uint32_t address)
{
    return address & BASE_MASK;
}

// Returns the index in the page directory for the given address.
static inline uint32_t _getPageDirectoryIndex(uint32_t address)
{
    return address >> (OFFSET_BITS + BITS_PER_ENTRY);
}

// Returns the index in the second level page table for the given address.
static inline uint32_t _getPageTableIndex(uint32_t address)
{
    return (address >> OFFSET_BITS) & ENTRY_MASK;
}

// Returns the offset within a page / frame for the given address.
static inline uint32_t _getOffset(uint32_t address)
{
    return address & OFFSET_MASK;
}

int mapPage(uint32_t virtualBase, uint32_t physicalBase, ReadWrite accessMode,
    PrivilegeLevel privileges)
{
    if ((_getOffset(virtualBase) != 0) || (_getOffset(physicalBase) != 0)) {
        return -1; // Invalid offset
    }

    assert(_cr3 != NULL);

    uint32_t pageTableIndex = _getPageTableIndex(virtualBase);
    uint32_t pageDirectoryIndex = _getPageDirectoryIndex(virtualBase);

    PageTable *pageT = NULL;

    if (!(_cr3->entries[pageDirectoryIndex] & PAGE_PRESENT_MASK)) {
        // Allocate a new page table
        if (posix_memalign((void **)&pageT, 4096, sizeof(PageTable)) != 0) {
            return -1; // Allocation failed
        }
        memset(pageT, 0, sizeof(PageTable)); // Initialize to zero
        _cr3->entries[pageDirectoryIndex] = pointerToInt(pageT) | PAGE_PRESENT_MASK;
    }

    pageT = (PageTable *)intToPointer(_cr3->entries[pageDirectoryIndex] & PAGE_DIRECTORY_ADDRESS_MASK);
    if (!pageT) {
        return -1;
    }

    // Set the PTE
    pageT->entries[pageTableIndex] = (physicalBase & PAGE_DIRECTORY_ADDRESS_MASK) | PAGE_PRESENT_MASK |
        (accessMode == ACCESS_WRITE ? PAGE_READWRITE_MASK : 0) |
        (privileges == USER_MODE ? PAGE_USERMODE_MASK : 0);

    return 0; // Success
}

#define TRANSLATION_ERROR 0xFFFFFFFF
// return 0xFFFFFFFF on error, physical frame number otherweise.
uint32_t translatePageTable(uint32_t virtualAddress, ReadWrite accessMode,
    PrivilegeLevel privileges)
{
    assert(_cr3 != NULL);
    // total bits 32; 10 page directory; 10 page table; 12 offset bits; 

    // Finna PT add, ef ekki til þá page falt, ef til þá á frame
    // Offsettið
    uint32_t pageTableIndex = _getPageTableIndex(virtualAddress);
    uint32_t pageDirectoryIndex = _getPageDirectoryIndex(virtualAddress);
    uint32_t offset = _getOffset(virtualAddress);
    // Finna PD add, ef ekki til þá page fault, ef til þá bendir á page table
    
    // Page table  = Page Directory[decimal af binary hlutanum af fyrstu 10 bitunum af virtual add]
    // Physical Add = Page Table[decimal af binary hlutanum af seinni 10 bitunum af virtual add] + offsettið
    if (!(_cr3->entries[pageDirectoryIndex] & PAGE_PRESENT_MASK)) {// 0x1
        return TRANSLATION_ERROR;
    } // Fail the translation if virtual address is not present in the page directory
    // or (if present) the corresponding page table;
    // We now know it exists
    PageTable *pageT = (PageTable *)intToPointer(_cr3->entries[pageDirectoryIndex] & PAGE_DIRECTORY_ADDRESS_MASK);
    if (!(pageT->entries[pageTableIndex] & PAGE_PRESENT_MASK)) {
        return TRANSLATION_ERROR;
    }

    // Fail the translation if the requested access mode is not permitted 
    // or if the CPU’s privilege level is insufficient.
    // EF pageT->entrie[pageTableIndex] er 0 þá ætti accessMode að vera bara READ only, ef bitinn er 1 þá ætti hann að vera Write
    if (accessMode == ACCESS_WRITE && privileges == USER_MODE && !(pageT->entries[pageTableIndex] & PAGE_READWRITE_MASK)) {
        return TRANSLATION_ERROR;
    }    

    if (privileges == USER_MODE && !(pageT->entries[pageTableIndex] & PAGE_USERMODE_MASK)) {
        return TRANSLATION_ERROR; 
    }
    //if all checks pass, perform the translation to compute the physical address, mark
    //the PTE as accessed by apdating its accessed flag, and return the translated physical address.
    pageT->entries[pageTableIndex] |= PAGE_ACCESSED_MASK;
    // Physical Add = Page Table[decimal af binary hlutanum af seinni 10 bitunum af virtual add] + offsettið
    uint32_t physicalFrame = pageT->entries[pageTableIndex] & PAGE_TABLE_ADDRESS_MASK;
    uint32_t compPhysicalAddr = physicalFrame + offset;
    return compPhysicalAddr;
}


// return -1 on error, 0 on success
int unmapPage(uint32_t virtualBase) {
        if (_getOffset(virtualBase) != 0) {
        return -1; // Invalid offset
    }

    assert(_cr3 != NULL);
    uint32_t pageTableIndex = _getPageTableIndex(virtualBase);
    uint32_t pageDirectoryIndex = _getPageDirectoryIndex(virtualBase);

    if (!(_cr3->entries[pageDirectoryIndex] & PAGE_PRESENT_MASK)) {
        return -1; // Page directory entry not present
    }

    PageTable *pageT = (PageTable *)intToPointer(_cr3->entries[pageDirectoryIndex] & PAGE_TABLE_ADDRESS_MASK);
    if (!pageT) {
        return -1; // Invalid page table
    }

    // Mark the page table entry as not present
    pageT->entries[pageTableIndex] &= ~PAGE_PRESENT_MASK;

    // Check if all entries in the page table are not present
    int allEntriesPresent = 0;
    for (int i = 0; i < ENTRIES_PER_TABLE; i++) {
        if (pageT->entries[i] & PAGE_PRESENT_MASK) {
            allEntriesPresent = 1;
            break;
        }
    }

    if (!allEntriesPresent) {
        // Free the page table
        free(pageT);
        // Mark the page directory entry as not present
        _cr3->entries[pageDirectoryIndex] &= ~PAGE_PRESENT_MASK;
    }

    return 0; // Success
}
