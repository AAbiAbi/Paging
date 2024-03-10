#include "page_table.h"
#include "memory_manager.h"
#include "address_file_processor.h"
#include "tlb.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

#define MEMORY_SIZE NUM_PAGES * PAGE_SIZE

int main(int argc, char* argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <addresses.txt>\n", argv[0]);
        return EXIT_FAILURE;
    }

    // Initialize the page table
    initialize_page_table();

    //initialize TLB
    initialize_tlb();

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
    printf("Success\n");
    // Cleanup
    munmap(mem, MEMORY_SIZE);
    close(fd);

    return EXIT_SUCCESS;
}

