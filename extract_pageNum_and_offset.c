#include <stdio.h>

void extract_page_number_and_offset(unsigned int logical_address) {
    // Extracting the page number by shifting the logical address right by 8 bits
    unsigned int page_number = (logical_address >> 8) & 0xFF;

    // Extracting the offset by masking the lower 8 bits of the logical address
    unsigned int offset = logical_address & 0xFF;

    printf("Logical Address: %u => Page Number: %u, Offset: %u\n", logical_address, page_number, offset);
}

int main() {
    // Array of given logical addresses
    unsigned int addresses[] = {1, 256, 32768, 32769, 128, 64434, 33153};
    int num_addresses = sizeof(addresses) / sizeof(addresses[0]);

    // Loop through the array and extract the page number and offset for each address
    for (int i = 0; i < num_addresses; i++) {
        extract_page_number_and_offset(addresses[i]);
    }

    return 0;
}
