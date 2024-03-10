#include <time.h>
#include "tlb.h"
#include <time.h>
#include <stdio.h>
#define TLB_ENTRIES 16
#define FRAME_NUMBER_NOT_FOUND -1

TLBEntry tlb[TLB_ENTRIES];

void initialize_tlb() {
    for (int i = 0; i < TLB_ENTRIES; i++) {
        tlb[i].page_number = 0;
        tlb[i].frame_number = FRAME_NUMBER_NOT_FOUND;
        tlb[i].last_accessed = 0;
        tlb[i].valid = 0; // Mark all entries as invalid initially
    }
    printf("TLB initialized\n");
}

int consult_tlb(unsigned int page_number) {
    int frame_number = FRAME_NUMBER_NOT_FOUND;
    time_t oldest_time = time(NULL);
    int lru_index = -1;

    for (int i = 0; i < TLB_ENTRIES; i++) {
        if (tlb[i].valid && tlb[i].page_number == page_number) {
            // TLB Hit
            printf("TLB hit: Page number %d is mapped to frame number %d.\n", page_number, tlb[i].frame_number);
            tlb[i].last_accessed = time(NULL); // Update the access time
            return tlb[i].frame_number;
        } else if (tlb[i].valid && (lru_index == -1 || tlb[i].last_accessed < oldest_time)) {
            // Track the least recently used entry
            oldest_time = tlb[i].last_accessed;
            lru_index = i;
        }
    }

    // TLB Miss, return -1 indicating that the page number was not found
    printf("TLB miss: Page number %d is not currently mapped to any frame.\n", page_number);
    return FRAME_NUMBER_NOT_FOUND;
}

void update_tlb(unsigned int page_number, int frame_number) {
    int lru_index = -1;
    time_t oldest_time = time(NULL);
    int found_invalid = 0; // Flag to indicate an invalid entry found

    for (int i = 0; i < TLB_ENTRIES; i++) {
        if (!tlb[i].valid) {
            lru_index = i;
            break; // Found an empty slot, break early
        } else if (tlb[i].last_accessed < oldest_time) {
            // Update LRU index
            oldest_time = tlb[i].last_accessed;
            lru_index = i;
        }
    }

    // If all entries were valid, we use the LRU index
    if (lru_index == -1) {
        // This should not happen if the for loop is correct
        lru_index = 0; // Fallback to overwrite the first entry (shouldn't get here)
    }

    // Update the TLB with the new entry
    tlb[lru_index].page_number = page_number;
    tlb[lru_index].frame_number = frame_number;
    tlb[lru_index].last_accessed = time(NULL);
    tlb[lru_index].valid = 1;
}

// ... Rest of your existing code...
