#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <time.h>
#include TLB.c


#define PAGE_SIZE 256
#define NUM_PAGES 256
#define TLB_ENTRIES 16
#define ADDRESS_SPACE_SIZE 65536
#define NUM_FRAMES NUM_PAGES
#define MEMORY_SIZE NUM_PAGES * PAGE_SIZE
#define FRAME_NUMBER_NOT_FOUND -1
#define FRAME_SIZE 256

int page_table[NUM_PAGES];

void initialize_page_table() {
    for (int i = 0; i < NUM_PAGES; i++) {
        page_table[i] = i; // Direct mapping for simplicity
    }
}

void translate_and_access_memory(unsigned int logical_address, unsigned char* mem) {
    // Extract page number and offset from the logical address
    unsigned int page_number = (logical_address >> 8) & 0xFF;
    unsigned int offset = logical_address & 0xFF;
    int frame_number = page_table[page_number];

    if (frame_number != FRAME_NUMBER_NOT_FOUND) {
        // Calculate the physical address
        unsigned int physical_address = (frame_number * PAGE_SIZE) + offset;
        // Read the value from the simulated physical memory
        char value = mem[physical_address];
        printf("Logical Address: %u, Physical Address: %u, Value: %d\n", logical_address, physical_address, value);
    } else {
        printf("Page fault at logical address: %u\n", logical_address);
    }
}

void process_addresses(const char* filename, unsigned char* mem) {
    FILE* file = fopen(filename, "r");
    if (!file) {
        perror("Failed to open file");
        exit(EXIT_FAILURE);
    }

    unsigned int logical_address;
    while (fscanf(file, "%u", &logical_address) != EOF) {
        translate_and_access_memory(logical_address, mem);
    }

    fclose(file);
}

// TLB table structure:Hashtable
//key of TLB table is page number and value is frame number amd time of access
// 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15




int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <addresses.txt>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Initialize the page table
    initialize_page_table();

    int fd = open("/dev/mem", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open /dev/mem");
        return EXIT_FAILURE;
    }

    // Map the "physical memory" space from /dev/mem
    off_t mem_offset = 0x100000; // Adjust this as necessary for your system
    unsigned char* mem = mmap(NULL, MEMORY_SIZE, PROT_READ, MAP_SHARED, fd, mem_offset);
    if (mem == MAP_FAILED) {
        perror("Failed to map memory");
        close(fd);
        return EXIT_FAILURE;
    }

    // Process the logical addresses and access simulated physical memory
    process_addresses(argv[1], mem);

    // Cleanup
    munmap(mem, MEMORY_SIZE);
    close(fd);

    return EXIT_SUCCESS;
}
