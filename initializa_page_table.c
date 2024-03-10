#include <stdio.h>
#include <stdlib.h>

#define PAGE_TABLE_SIZE 256
#define FRAME_NUMBER_NOT_FOUND -1

// Declare the page table as a global variable for simplicity.
int page_table[PAGE_TABLE_SIZE];

void initialize_page_table() {
    for(int i = 0; i < PAGE_TABLE_SIZE; i++) {
        page_table[i] = FRAME_NUMBER_NOT_FOUND; // Initialize all entries to -1.
    }
}

// Function to simulate accessing a page and checking for a hit or miss
void access_page(int page_number) {
    if(page_number < 0 || page_number >= PAGE_TABLE_SIZE) {
        printf("Page number %d is out of bounds.\n", page_number);
        return;
    }
    
    int frame_number = page_table[page_number];
    if(frame_number != FRAME_NUMBER_NOT_FOUND) {
        // Page hit
        printf("Page hit: Page number %d is mapped to frame number %d.\n", page_number, frame_number);
    } else {
        // Page miss
        printf("Page miss: Page number %d is not currently mapped to any frame.\n", page_number);
        // Here, you would typically trigger a page fault handler to load the page.
        // For simplicity, we'll just simulate this by assigning a new frame number (for demonstration only).
        // In a real system, you would need to select a frame based on your memory management strategy.
        int new_frame_number = (page_number % PAGE_TABLE_SIZE); // Simplistic and for demonstration only
        page_table[page_number] = new_frame_number;
        printf("Simulating page load: Mapping page number %d to frame number %d.\n", page_number, new_frame_number);
    }

}

int main() {
    // Initialize the page table
    initialize_page_table();

    // Example: Assume we want to map page number 5 to frame number 10
    int example_page_number = 5;
    int example_frame_number = 10;

    // Update the page table with the mapping
    page_table[example_page_number] = example_frame_number;

    // For demonstration, let's print the frame number for page number 5
    printf("Frame number for page %d: %d\n", example_page_number, page_table[example_page_number]);

    // Similarly, you can update the page table with actual mappings based on your memory management logic
    // and access the frame number using the page number as an index into the page table array.

    return 0;
}
