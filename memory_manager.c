#include "memory_manager.h"
#include "tlb.h"
#include "page_table.h"
#include <stdio.h>

// ... (rest of the memory_manager functions)
void translate_and_access_memory(unsigned int logical_address, unsigned char* mem) {

    FILE* file = fopen("output.txt", "a");
    if (file == NULL) {
        perror("Failed to open output file");
        return; // Exit the function if file opening fails
    }
    
    // Extract page number and offset from the logical address
    unsigned int page_number = (logical_address >> 8) & 0xFF;
    unsigned int offset = logical_address & 0xFF;
    // First, attempt to get the frame number from the TLB
    int frame_number = consult_tlb(page_number);

    // If frame number is not found in TLB, then consult the page table
    if (frame_number == FRAME_NUMBER_NOT_FOUND) {
        frame_number = get_frame_number_from_page_table(page_number);
       
    }

    // Handle the case where the frame number is not found
    if (frame_number == FRAME_NUMBER_NOT_FOUND) {
        char value1 = mem[logical_address];
        // Handle the page fault here (e.g., by loading the frame into memory)
        fprintf(file, "Page fault at logical address: %u\n", value1);
        // You can simulate loading the frame into memory and updating the page table here
        // For now, let's just simulate it by assigning a frame number
        frame_number = page_number; // Simple simulation for example purposes
        update_page_table(page_number, frame_number);
        update_tlb(page_number, frame_number);
    }

    // At this point, frame_number should be valid and can be used to calculate the physical address
    unsigned int physical_address = (frame_number * PAGE_SIZE) + offset;
    char value = mem[physical_address];
    fprintf(file, "Logical Address: %u, Physical Address: %u, Value: %d\n", logical_address, physical_address, value);

    // Update the TLB with the new frame number
    update_tlb(page_number, frame_number);
}
