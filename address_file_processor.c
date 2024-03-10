#include "address_file_processor.h"
#include "memory_manager.h"
#include <stdio.h>
#include <stdlib.h>

// ... (rest of the address_file_processor functions)

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