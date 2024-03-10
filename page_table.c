#include "page_table.h"

// Global page table array
int page_table[NUM_PAGES];

void initialize_page_table() {
    for (int i = 0; i < NUM_PAGES; i++) {
        page_table[i] = i; // Direct mapping for simplicity
    }
}

int get_frame_number_from_page_table(unsigned int page_number) {
    // Implementation to get the frame number from the page table
    //also upate the tlb
    
    return page_table[page_number];
}

void update_page_table(unsigned int page_number, int frame_number) {
    // Implementation to update the page table
    // For a simple direct-mapped page table, it might look like this:
    page_table[page_number] = frame_number;
}