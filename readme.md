# Virtual Memory Manager

Github:https://github.com/AAbiAbi/Paging.git

Designing a Virtual Memory Manager for translating logical addresses to physical addresses is a foundational exercise in understanding how operating systems manage memory. Here's a step-by-step guide to help you structure your program, followed by a simple C code example to get you started.

This diagram shows the flow from memory accesses to the TLB, which either results in hits (leading directly to physical memory access) or misses (requiring a lookup in the page table before accessing physical memory). The percentages represent the proportion of hits and misses observed in your simulation.

```
[ Accesses ]  ->  [ TLB: 16 entries ]
                     |        |     
                    /          \
             Hit (4.9%)      Miss (95.1%)
                    \          /
                     V        V
                  [ Page Table ]
                       |
                       V
                  [ Physical Memory ]

```

## Step 1: Understand the Basics
Virtual Address Space: 65,536 bytes.
Logical Addresses: 32-bit integers, but you'll use only the rightmost 16 bits.
Page Number and Offset: The 16-bit address is split into an 8-bit page number and an 8-bit page offset.
Page Table Entries: 2^8 entries.
Page Size: 256 bytes.
TLB Entries: 16.
Frame Size: 256 bytes.
Number of Frames: 256.
Physical Memory: 65,536 bytes.
## Step 2: Extract Page Number and Offset
Use bit-masking and bit-shifting to extract the page number and offset from the logical address.

```c

unsigned int page_number = (logical_address >> 8) & 0xFF;
unsigned int offset = logical_address & 0xFF;
```
```c
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

```

OutPut:
```sh
abi@abi-virtual-machine:~/Project4/Paging$ cd "/home/abi/Project4/Paging/" && gcc extract_pageNum_and_offset.c -o extract_pageNum_and_offset && "/home/abi/Project4/Paging/"extract_pageNum_and_offset
Logical Address: 1 => Page Number: 0, Offset: 1
Logical Address: 256 => Page Number: 1, Offset: 0
Logical Address: 32768 => Page Number: 128, Offset: 0
Logical Address: 32769 => Page Number: 128, Offset: 1
Logical Address: 128 => Page Number: 0, Offset: 128
Logical Address: 64434 => Page Number: 251, Offset: 178
Logical Address: 33153 => Page Number: 129, Offset: 129

```

## Step 3: Initialize Data Structures

Initially, we suggest that you bypass the TLB and use only a page table

You need to initialize the page table and the TLB. Optionally, you can start without the TLB and add it later.

```c

int page_table[256]; // Initialize to -1 to indicate that no pages are loaded.
int tlb[16][2]; // First column for page numbers, second for frame numbers. Initialize appropriately.
```
## Step 4: Address Translation
Without TLB: Directly use the page table to find the frame number.
With TLB: First, check the TLB for a quicker lookup. If the page number is not in the TLB, use the page table.

```c
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

```
## Step 5: TLB Management
Implement a replacement strategy (LRU) for when the TLB is full and needs updating.

Using LRU with t_time, but if met a vacant entry, newcomer tlb entry will occupy the vacant first.

```c
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
```


## Step 6: Output Results

### Running command
```sh
##Optional
gcc project4.c page_table.c tlb.c memory_manager.
c address_file_processor.c -o a.out

## must have sudo to access the linux kernel
sudo "/home/abi/Project4/Paging/a.out" /home/abi/
Project4/Paging/address.txt
```


```sh
abi@abi-virtual-machine:~/Project4/Paging$ sudo "/home/abi/Project4/Paging/vm_simulator" /home/abi/Project4/Paging/address.txt
TLB initialized
TLB miss: Page number 63 is not currently mapped to any frame.
TLB miss: Page number 48 is not currently mapped to any frame.
TLB miss: Page number 160 is not currently mapped to any frame.
TLB miss: Page number 80 is not currently mapped to any frame.
TLB miss: Page number 133 is not currently mapped to any frame.
TLB miss: Page number 162 is not currently mapped to any frame.
TLB miss: Page number 143 is not currently mapped to any frame.
TLB miss: Page number 200 is not currently mapped to any frame.
TLB miss: Page number 28 is not currently mapped to any frame.
TLB miss: Page number 33 is not currently mapped to any frame.
TLB miss: Page number 138 is not currently mapped to any frame.
TLB miss: Page number 146 is not currently mapped to any frame.
TLB miss: Page number 192 is not currently mapped to any frame.
TLB miss: Page number 53 is not currently mapped to any frame.
TLB miss: Page number 215 is not currently mapped to any frame.
TLB miss: Page number 104 is not currently mapped to any frame.
TLB miss: Page number 181 is not currently mapped to any frame.
TLB miss: Page number 85 is not currently mapped to any frame.
TLB miss: Page number 117 is not currently mapped to any frame.
TLB miss: Page number 211 is not currently mapped to any frame.
TLB miss: Page number 251 is not currently mapped to any frame.
TLB miss: Page number 199 is not currently mapped to any frame.
TLB miss: Page number 99 is not currently mapped to any frame.
TLB miss: Page number 76 is not currently mapped to any frame.
TLB miss: Page number 78 is not currently mapped to any frame.
TLB miss: Page number 70 is not currently mapped to any frame.
TLB miss: Page number 95 is not currently mapped to any frame.
TLB miss: Page number 21 is not currently mapped to any frame.
TLB miss: Page number 242 is not currently mapped to any frame.
TLB miss: Page number 22 is not currently mapped to any frame.
TLB miss: Page number 171 is not currently mapped to any frame.
TLB miss: Page number 47 is not currently mapped to any frame.
TLB miss: Page number 231 is not currently mapped to any frame.
TLB miss: Page number 55 is not currently mapped to any frame.
TLB miss: Page number 73 is not currently mapped to any frame.
TLB miss: Page number 86 is not currently mapped to any frame.
TLB miss: Page number 45 is not currently mapped to any frame.
TLB miss: Page number 168 is not currently mapped to any frame.
TLB miss: Page number 55 is not currently mapped to any frame.
TLB miss: Page number 207 is not currently mapped to any frame.
TLB miss: Page number 129 is not currently mapped to any frame.
TLB miss: Page number 107 is not currently mapped to any frame.
TLB miss: Page number 51 is not currently mapped to any frame.
TLB miss: Page number 152 is not currently mapped to any frame.
TLB miss: Page number 105 is not currently mapped to any frame.
TLB miss: Page number 59 is not currently mapped to any frame.
TLB miss: Page number 127 is not currently mapped to any frame.
TLB miss: Page number 178 is not currently mapped to any frame.
TLB miss: Page number 94 is not currently mapped to any frame.
TLB miss: Page number 92 is not currently mapped to any frame.
TLB miss: Page number 29 is not currently mapped to any frame.
TLB miss: Page number 218 is not currently mapped to any frame.
TLB miss: Page number 96 is not currently mapped to any frame.
TLB miss: Page number 206 is not currently mapped to any frame.
TLB miss: Page number 12 is not currently mapped to any frame.
TLB miss: Page number 82 is not currently mapped to any frame.
TLB miss: Page number 247 is not currently mapped to any frame.
TLB miss: Page number 77 is not currently mapped to any frame.
TLB miss: Page number 7 is not currently mapped to any frame.
TLB miss: Page number 27 is not currently mapped to any frame.
TLB miss: Page number 155 is not currently mapped to any frame.
TLB miss: Page number 96 is not currently mapped to any frame.
TLB miss: Page number 84 is not currently mapped to any frame.
TLB miss: Page number 54 is not currently mapped to any frame.
TLB miss: Page number 144 is not currently mapped to any frame.
TLB miss: Page number 129 is not currently mapped to any frame.
TLB miss: Page number 241 is not currently mapped to any frame.
TLB miss: Page number 93 is not currently mapped to any frame.
TLB miss: Page number 214 is not currently mapped to any frame.
TLB miss: Page number 243 is not currently mapped to any frame.
TLB miss: Page number 224 is not currently mapped to any frame.
TLB miss: Page number 25 is not currently mapped to any frame.
TLB miss: Page number 41 is not currently mapped to any frame.
TLB miss: Page number 114 is not currently mapped to any frame.
TLB miss: Page number 219 is not currently mapped to any frame.
TLB miss: Page number 227 is not currently mapped to any frame.
TLB miss: Page number 17 is not currently mapped to any frame.
TLB miss: Page number 66 is not currently mapped to any frame.
TLB miss: Page number 40 is not currently mapped to any frame.
TLB miss: Page number 185 is not currently mapped to any frame.
TLB miss: Page number 6 is not currently mapped to any frame.
TLB miss: Page number 98 is not currently mapped to any frame.
TLB miss: Page number 198 is not currently mapped to any frame.
TLB miss: Page number 36 is not currently mapped to any frame.
TLB miss: Page number 8 is not currently mapped to any frame.
TLB miss: Page number 212 is not currently mapped to any frame.
TLB miss: Page number 112 is not currently mapped to any frame.
TLB miss: Page number 157 is not currently mapped to any frame.
TLB miss: Page number 159 is not currently mapped to any frame.
TLB miss: Page number 151 is not currently mapped to any frame.
TLB miss: Page number 176 is not currently mapped to any frame.
TLB miss: Page number 102 is not currently mapped to any frame.
TLB miss: Page number 67 is not currently mapped to any frame.
TLB miss: Page number 103 is not currently mapped to any frame.
TLB miss: Page number 252 is not currently mapped to any frame.
TLB miss: Page number 60 is not currently mapped to any frame.
TLB miss: Page number 56 is not currently mapped to any frame.
TLB miss: Page number 136 is not currently mapped to any frame.
TLB miss: Page number 154 is not currently mapped to any frame.
TLB miss: Page number 167 is not currently mapped to any frame.
TLB miss: Page number 126 is not currently mapped to any frame.
TLB miss: Page number 249 is not currently mapped to any frame.
TLB miss: Page number 68 is not currently mapped to any frame.
TLB miss: Page number 32 is not currently mapped to any frame.
TLB miss: Page number 210 is not currently mapped to any frame.
TLB miss: Page number 120 is not currently mapped to any frame.
TLB miss: Page number 132 is not currently mapped to any frame.
TLB miss: Page number 233 is not currently mapped to any frame.
TLB miss: Page number 118 is not currently mapped to any frame.
TLB miss: Page number 140 is not currently mapped to any frame.
TLB miss: Page number 208 is not currently mapped to any frame.
TLB miss: Page number 3 is not currently mapped to any frame.
TLB miss: Page number 107 is not currently mapped to any frame.
TLB miss: Page number 173 is not currently mapped to any frame.
TLB miss: Page number 110 is not currently mapped to any frame.
TLB miss: Page number 43 is not currently mapped to any frame.
TLB miss: Page number 177 is not currently mapped to any frame.
TLB miss: Page number 31 is not currently mapped to any frame.
TLB miss: Page number 25 is not currently mapped to any frame.
TLB miss: Page number 163 is not currently mapped to any frame.
TLB miss: Page number 72 is not currently mapped to any frame.
TLB miss: Page number 158 is not currently mapped to any frame.
TLB miss: Page number 88 is not currently mapped to any frame.
TLB miss: Page number 108 is not currently mapped to any frame.
TLB miss: Page number 236 is not currently mapped to any frame.
TLB miss: Page number 109 is not currently mapped to any frame.
TLB miss: Page number 165 is not currently mapped to any frame.
TLB miss: Page number 135 is not currently mapped to any frame.
TLB miss: Page number 225 is not currently mapped to any frame.
TLB miss: Page number 210 is not currently mapped to any frame.
TLB miss: Page number 66 is not currently mapped to any frame.
TLB miss: Page number 174 is not currently mapped to any frame.
TLB miss: Page number 39 is not currently mapped to any frame.
TLB miss: Page number 0 is not currently mapped to any frame.
TLB miss: Page number 237 is not currently mapped to any frame.
TLB miss: Page number 38 is not currently mapped to any frame.
TLB miss: Page number 122 is not currently mapped to any frame.
TLB miss: Page number 195 is not currently mapped to any frame.
TLB miss: Page number 44 is not currently mapped to any frame.
TLB miss: Page number 240 is not currently mapped to any frame.
TLB miss: Page number 30 is not currently mapped to any frame.
TLB miss: Page number 147 is not currently mapped to any frame.
TLB miss: Page number 121 is not currently mapped to any frame.
TLB miss: Page number 10 is not currently mapped to any frame.
TLB miss: Page number 19 is not currently mapped to any frame.
TLB miss: Page number 179 is not currently mapped to any frame.
TLB miss: Page number 23 is not currently mapped to any frame.
TLB miss: Page number 49 is not currently mapped to any frame.
TLB miss: Page number 89 is not currently mapped to any frame.
TLB miss: Page number 37 is not currently mapped to any frame.
TLB miss: Page number 193 is not currently mapped to any frame.
TLB miss: Page number 238 is not currently mapped to any frame.
TLB miss: Page number 240 is not currently mapped to any frame.
TLB miss: Page number 232 is not currently mapped to any frame.
TLB miss: Page number 1 is not currently mapped to any frame.
TLB miss: Page number 115 is not currently mapped to any frame.
TLB miss: Page number 148 is not currently mapped to any frame.
TLB miss: Page number 184 is not currently mapped to any frame.
TLB miss: Page number 77 is not currently mapped to any frame.
TLB miss: Page number 141 is not currently mapped to any frame.
TLB miss: Page number 137 is not currently mapped to any frame.
TLB miss: Page number 217 is not currently mapped to any frame.
TLB miss: Page number 235 is not currently mapped to any frame.
TLB miss: Page number 248 is not currently mapped to any frame.
TLB miss: Page number 207 is not currently mapped to any frame.
TLB miss: Page number 74 is not currently mapped to any frame.
TLB miss: Page number 149 is not currently mapped to any frame.
TLB miss: Page number 113 is not currently mapped to any frame.
TLB miss: Page number 35 is not currently mapped to any frame.
TLB miss: Page number 201 is not currently mapped to any frame.
TLB miss: Page number 213 is not currently mapped to any frame.
TLB miss: Page number 58 is not currently mapped to any frame.
TLB miss: Page number 199 is not currently mapped to any frame.
TLB miss: Page number 221 is not currently mapped to any frame.
TLB hit: Page number 33 is mapped to frame number 33.
TLB miss: Page number 16 is not currently mapped to any frame.
TLB miss: Page number 188 is not currently mapped to any frame.
TLB miss: Page number 254 is not currently mapped to any frame.
TLB miss: Page number 123 is not currently mapped to any frame.
TLB miss: Page number 124 is not currently mapped to any frame.
TLB miss: Page number 2 is not currently mapped to any frame.
TLB miss: Page number 44 is not currently mapped to any frame.
TLB miss: Page number 106 is not currently mapped to any frame.
TLB miss: Page number 244 is not currently mapped to any frame.
TLB miss: Page number 85 is not currently mapped to any frame.
TLB miss: Page number 239 is not currently mapped to any frame.
TLB miss: Page number 101 is not currently mapped to any frame.
TLB miss: Page number 189 is not currently mapped to any frame.
TLB miss: Page number 61 is not currently mapped to any frame.
TLB miss: Page number 190 is not currently mapped to any frame.
TLB miss: Page number 246 is not currently mapped to any frame.
TLB miss: Page number 216 is not currently mapped to any frame.
TLB miss: Page number 50 is not currently mapped to any frame.
TLB miss: Page number 223 is not currently mapped to any frame.
TLB miss: Page number 79 is not currently mapped to any frame.
TLB miss: Page number 255 is not currently mapped to any frame.
TLB miss: Page number 52 is not currently mapped to any frame.
TLB miss: Page number 81 is not currently mapped to any frame.
TLB miss: Page number 62 is not currently mapped to any frame.
TLB miss: Page number 119 is not currently mapped to any frame.
TLB miss: Page number 203 is not currently mapped to any frame.
TLB miss: Page number 20 is not currently mapped to any frame.
TLB miss: Page number 57 is not currently mapped to any frame.
TLB miss: Page number 65 is not currently mapped to any frame.
TLB miss: Page number 111 is not currently mapped to any frame.
TLB miss: Page number 228 is not currently mapped to any frame.
TLB miss: Page number 202 is not currently mapped to any frame.
TLB miss: Page number 91 is not currently mapped to any frame.
TLB miss: Page number 100 is not currently mapped to any frame.
TLB miss: Page number 4 is not currently mapped to any frame.
TLB miss: Page number 36 is not currently mapped to any frame.
TLB miss: Page number 130 is not currently mapped to any frame.
TLB miss: Page number 170 is not currently mapped to any frame.
TLB miss: Page number 118 is not currently mapped to any frame.
TLB miss: Page number 18 is not currently mapped to any frame.
TLB miss: Page number 251 is not currently mapped to any frame.
TLB miss: Page number 253 is not currently mapped to any frame.
TLB miss: Page number 245 is not currently mapped to any frame.
TLB miss: Page number 14 is not currently mapped to any frame.
TLB miss: Page number 196 is not currently mapped to any frame.
TLB miss: Page number 229 is not currently mapped to any frame.
TLB miss: Page number 9 is not currently mapped to any frame.
TLB miss: Page number 90 is not currently mapped to any frame.
TLB miss: Page number 222 is not currently mapped to any frame.
TLB miss: Page number 218 is not currently mapped to any frame.
TLB miss: Page number 42 is not currently mapped to any frame.
TLB miss: Page number 248 is not currently mapped to any frame.
TLB miss: Page number 5 is not currently mapped to any frame.
TLB miss: Page number 220 is not currently mapped to any frame.
TLB miss: Page number 87 is not currently mapped to any frame.
TLB miss: Page number 230 is not currently mapped to any frame.
TLB miss: Page number 126 is not currently mapped to any frame.
TLB miss: Page number 115 is not currently mapped to any frame.
TLB miss: Page number 26 is not currently mapped to any frame.
TLB miss: Page number 226 is not currently mapped to any frame.
TLB miss: Page number 71 is not currently mapped to any frame.
TLB miss: Page number 24 is not currently mapped to any frame.
TLB miss: Page number 234 is not currently mapped to any frame.
TLB miss: Page number 46 is not currently mapped to any frame.
TLB miss: Page number 97 is not currently mapped to any frame.
TLB miss: Page number 13 is not currently mapped to any frame.
TLB miss: Page number 11 is not currently mapped to any frame.
TLB miss: Page number 204 is not currently mapped to any frame.
TLB miss: Page number 205 is not currently mapped to any frame.
TLB miss: Page number 83 is not currently mapped to any frame.
TLB miss: Page number 125 is not currently mapped to any frame.
TLB miss: Page number 187 is not currently mapped to any frame.
TLB miss: Page number 69 is not currently mapped to any frame.
TLB miss: Page number 166 is not currently mapped to any frame.
TLB miss: Page number 64 is not currently mapped to any frame.
TLB miss: Page number 182 is not currently mapped to any frame.
TLB miss: Page number 14 is not currently mapped to any frame.
TLB miss: Page number 74 is not currently mapped to any frame.
TLB miss: Page number 15 is not currently mapped to any frame.
TLB miss: Page number 3 is not currently mapped to any frame.
TLB miss: Page number 229 is not currently mapped to any frame.
TLB miss: Page number 63 is not currently mapped to any frame.
TLB hit: Page number 48 is mapped to frame number 48.
TLB hit: Page number 160 is mapped to frame number 160.
TLB hit: Page number 80 is mapped to frame number 80.
TLB hit: Page number 133 is mapped to frame number 133.
TLB hit: Page number 162 is mapped to frame number 162.
TLB hit: Page number 143 is mapped to frame number 143.
TLB hit: Page number 200 is mapped to frame number 200.
TLB hit: Page number 28 is mapped to frame number 28.
TLB hit: Page number 33 is mapped to frame number 33.
TLB hit: Page number 138 is mapped to frame number 138.
TLB hit: Page number 146 is mapped to frame number 146.
TLB hit: Page number 192 is mapped to frame number 192.
TLB hit: Page number 53 is mapped to frame number 53.
TLB hit: Page number 215 is mapped to frame number 215.
TLB hit: Page number 104 is mapped to frame number 104.
TLB miss: Page number 181 is not currently mapped to any frame.
TLB miss: Page number 85 is not currently mapped to any frame.
TLB miss: Page number 117 is not currently mapped to any frame.
TLB miss: Page number 211 is not currently mapped to any frame.
TLB miss: Page number 251 is not currently mapped to any frame.
TLB miss: Page number 199 is not currently mapped to any frame.
TLB miss: Page number 99 is not currently mapped to any frame.
TLB miss: Page number 76 is not currently mapped to any frame.
TLB miss: Page number 78 is not currently mapped to any frame.
TLB miss: Page number 70 is not currently mapped to any frame.
TLB miss: Page number 95 is not currently mapped to any frame.
TLB miss: Page number 21 is not currently mapped to any frame.
TLB miss: Page number 242 is not currently mapped to any frame.
TLB miss: Page number 22 is not currently mapped to any frame.
TLB miss: Page number 171 is not currently mapped to any frame.
TLB miss: Page number 47 is not currently mapped to any frame.
TLB miss: Page number 231 is not currently mapped to any frame.
TLB miss: Page number 55 is not currently mapped to any frame.
TLB miss: Page number 73 is not currently mapped to any frame.
TLB miss: Page number 86 is not currently mapped to any frame.
TLB miss: Page number 45 is not currently mapped to any frame.
TLB miss: Page number 168 is not currently mapped to any frame.
TLB miss: Page number 55 is not currently mapped to any frame.
TLB miss: Page number 207 is not currently mapped to any frame.
TLB miss: Page number 129 is not currently mapped to any frame.
TLB miss: Page number 107 is not currently mapped to any frame.
TLB miss: Page number 51 is not currently mapped to any frame.
TLB miss: Page number 152 is not currently mapped to any frame.
TLB miss: Page number 105 is not currently mapped to any frame.
TLB miss: Page number 59 is not currently mapped to any frame.
TLB miss: Page number 127 is not currently mapped to any frame.
TLB miss: Page number 178 is not currently mapped to any frame.
TLB miss: Page number 94 is not currently mapped to any frame.
TLB miss: Page number 92 is not currently mapped to any frame.
TLB miss: Page number 29 is not currently mapped to any frame.
TLB miss: Page number 218 is not currently mapped to any frame.
TLB miss: Page number 96 is not currently mapped to any frame.
TLB miss: Page number 206 is not currently mapped to any frame.
TLB miss: Page number 12 is not currently mapped to any frame.
TLB miss: Page number 82 is not currently mapped to any frame.
TLB miss: Page number 247 is not currently mapped to any frame.
TLB miss: Page number 77 is not currently mapped to any frame.
TLB miss: Page number 7 is not currently mapped to any frame.
TLB miss: Page number 27 is not currently mapped to any frame.
TLB miss: Page number 155 is not currently mapped to any frame.
TLB miss: Page number 96 is not currently mapped to any frame.
TLB miss: Page number 84 is not currently mapped to any frame.
TLB miss: Page number 54 is not currently mapped to any frame.
TLB miss: Page number 144 is not currently mapped to any frame.
TLB miss: Page number 129 is not currently mapped to any frame.
TLB miss: Page number 241 is not currently mapped to any frame.
TLB miss: Page number 93 is not currently mapped to any frame.
TLB miss: Page number 214 is not currently mapped to any frame.
TLB miss: Page number 243 is not currently mapped to any frame.
TLB miss: Page number 224 is not currently mapped to any frame.
TLB miss: Page number 25 is not currently mapped to any frame.
TLB miss: Page number 41 is not currently mapped to any frame.
TLB miss: Page number 114 is not currently mapped to any frame.
TLB miss: Page number 219 is not currently mapped to any frame.
TLB miss: Page number 227 is not currently mapped to any frame.
TLB miss: Page number 17 is not currently mapped to any frame.
TLB miss: Page number 66 is not currently mapped to any frame.
TLB miss: Page number 40 is not currently mapped to any frame.
TLB miss: Page number 185 is not currently mapped to any frame.
TLB miss: Page number 6 is not currently mapped to any frame.
TLB miss: Page number 98 is not currently mapped to any frame.
TLB miss: Page number 198 is not currently mapped to any frame.
TLB miss: Page number 36 is not currently mapped to any frame.
TLB miss: Page number 8 is not currently mapped to any frame.
TLB miss: Page number 212 is not currently mapped to any frame.
TLB miss: Page number 112 is not currently mapped to any frame.
TLB miss: Page number 157 is not currently mapped to any frame.
TLB miss: Page number 159 is not currently mapped to any frame.
TLB miss: Page number 151 is not currently mapped to any frame.
TLB miss: Page number 176 is not currently mapped to any frame.
TLB miss: Page number 102 is not currently mapped to any frame.
TLB miss: Page number 67 is not currently mapped to any frame.
TLB miss: Page number 103 is not currently mapped to any frame.
TLB miss: Page number 252 is not currently mapped to any frame.
TLB miss: Page number 60 is not currently mapped to any frame.
TLB miss: Page number 56 is not currently mapped to any frame.
TLB miss: Page number 136 is not currently mapped to any frame.
TLB miss: Page number 154 is not currently mapped to any frame.
TLB miss: Page number 167 is not currently mapped to any frame.
TLB miss: Page number 126 is not currently mapped to any frame.
TLB miss: Page number 249 is not currently mapped to any frame.
TLB miss: Page number 68 is not currently mapped to any frame.
TLB miss: Page number 32 is not currently mapped to any frame.
TLB miss: Page number 210 is not currently mapped to any frame.
TLB miss: Page number 120 is not currently mapped to any frame.
TLB miss: Page number 132 is not currently mapped to any frame.
TLB miss: Page number 233 is not currently mapped to any frame.
TLB miss: Page number 118 is not currently mapped to any frame.
TLB miss: Page number 140 is not currently mapped to any frame.
TLB miss: Page number 208 is not currently mapped to any frame.
TLB miss: Page number 3 is not currently mapped to any frame.
TLB miss: Page number 107 is not currently mapped to any frame.
TLB miss: Page number 173 is not currently mapped to any frame.
TLB miss: Page number 110 is not currently mapped to any frame.
TLB miss: Page number 43 is not currently mapped to any frame.
TLB miss: Page number 177 is not currently mapped to any frame.
TLB miss: Page number 31 is not currently mapped to any frame.
TLB miss: Page number 25 is not currently mapped to any frame.
TLB miss: Page number 163 is not currently mapped to any frame.
TLB miss: Page number 72 is not currently mapped to any frame.
TLB miss: Page number 158 is not currently mapped to any frame.
TLB miss: Page number 88 is not currently mapped to any frame.
TLB miss: Page number 108 is not currently mapped to any frame.
TLB miss: Page number 236 is not currently mapped to any frame.
TLB miss: Page number 109 is not currently mapped to any frame.
TLB miss: Page number 165 is not currently mapped to any frame.
TLB miss: Page number 135 is not currently mapped to any frame.
TLB miss: Page number 225 is not currently mapped to any frame.
TLB miss: Page number 210 is not currently mapped to any frame.
TLB miss: Page number 66 is not currently mapped to any frame.
TLB miss: Page number 174 is not currently mapped to any frame.
TLB miss: Page number 39 is not currently mapped to any frame.
TLB miss: Page number 0 is not currently mapped to any frame.
TLB miss: Page number 237 is not currently mapped to any frame.
TLB miss: Page number 38 is not currently mapped to any frame.
TLB miss: Page number 122 is not currently mapped to any frame.
TLB miss: Page number 195 is not currently mapped to any frame.
TLB miss: Page number 44 is not currently mapped to any frame.
TLB miss: Page number 240 is not currently mapped to any frame.
TLB miss: Page number 30 is not currently mapped to any frame.
TLB miss: Page number 147 is not currently mapped to any frame.
TLB miss: Page number 121 is not currently mapped to any frame.
TLB miss: Page number 10 is not currently mapped to any frame.
TLB miss: Page number 19 is not currently mapped to any frame.
TLB miss: Page number 179 is not currently mapped to any frame.
TLB miss: Page number 23 is not currently mapped to any frame.
TLB miss: Page number 49 is not currently mapped to any frame.
TLB miss: Page number 89 is not currently mapped to any frame.
TLB miss: Page number 37 is not currently mapped to any frame.
TLB miss: Page number 193 is not currently mapped to any frame.
TLB miss: Page number 238 is not currently mapped to any frame.
TLB miss: Page number 240 is not currently mapped to any frame.
TLB miss: Page number 232 is not currently mapped to any frame.
TLB miss: Page number 1 is not currently mapped to any frame.
TLB miss: Page number 115 is not currently mapped to any frame.
TLB miss: Page number 148 is not currently mapped to any frame.
TLB miss: Page number 184 is not currently mapped to any frame.
TLB miss: Page number 77 is not currently mapped to any frame.
TLB miss: Page number 141 is not currently mapped to any frame.
TLB miss: Page number 137 is not currently mapped to any frame.
TLB miss: Page number 217 is not currently mapped to any frame.
TLB miss: Page number 235 is not currently mapped to any frame.
TLB miss: Page number 248 is not currently mapped to any frame.
TLB miss: Page number 207 is not currently mapped to any frame.
TLB miss: Page number 74 is not currently mapped to any frame.
TLB miss: Page number 149 is not currently mapped to any frame.
TLB miss: Page number 113 is not currently mapped to any frame.
TLB miss: Page number 35 is not currently mapped to any frame.
TLB miss: Page number 201 is not currently mapped to any frame.
TLB miss: Page number 213 is not currently mapped to any frame.
TLB miss: Page number 58 is not currently mapped to any frame.
TLB miss: Page number 199 is not currently mapped to any frame.
TLB miss: Page number 221 is not currently mapped to any frame.
TLB hit: Page number 33 is mapped to frame number 33.
TLB miss: Page number 16 is not currently mapped to any frame.
TLB miss: Page number 188 is not currently mapped to any frame.
TLB miss: Page number 254 is not currently mapped to any frame.
TLB miss: Page number 123 is not currently mapped to any frame.
TLB miss: Page number 124 is not currently mapped to any frame.
TLB miss: Page number 2 is not currently mapped to any frame.
TLB miss: Page number 44 is not currently mapped to any frame.
TLB miss: Page number 106 is not currently mapped to any frame.
TLB miss: Page number 244 is not currently mapped to any frame.
TLB miss: Page number 85 is not currently mapped to any frame.
TLB miss: Page number 239 is not currently mapped to any frame.
TLB miss: Page number 101 is not currently mapped to any frame.
TLB miss: Page number 189 is not currently mapped to any frame.
TLB miss: Page number 61 is not currently mapped to any frame.
TLB miss: Page number 190 is not currently mapped to any frame.
TLB miss: Page number 246 is not currently mapped to any frame.
TLB miss: Page number 216 is not currently mapped to any frame.
TLB miss: Page number 50 is not currently mapped to any frame.
TLB miss: Page number 223 is not currently mapped to any frame.
TLB miss: Page number 79 is not currently mapped to any frame.
TLB miss: Page number 255 is not currently mapped to any frame.
TLB miss: Page number 52 is not currently mapped to any frame.
TLB miss: Page number 81 is not currently mapped to any frame.
TLB miss: Page number 62 is not currently mapped to any frame.
TLB miss: Page number 119 is not currently mapped to any frame.
TLB miss: Page number 203 is not currently mapped to any frame.
TLB miss: Page number 20 is not currently mapped to any frame.
TLB miss: Page number 57 is not currently mapped to any frame.
TLB miss: Page number 65 is not currently mapped to any frame.
TLB miss: Page number 111 is not currently mapped to any frame.
TLB miss: Page number 228 is not currently mapped to any frame.
TLB miss: Page number 202 is not currently mapped to any frame.
TLB miss: Page number 91 is not currently mapped to any frame.
TLB miss: Page number 100 is not currently mapped to any frame.
TLB miss: Page number 4 is not currently mapped to any frame.
TLB miss: Page number 36 is not currently mapped to any frame.
TLB miss: Page number 130 is not currently mapped to any frame.
TLB miss: Page number 170 is not currently mapped to any frame.
TLB miss: Page number 118 is not currently mapped to any frame.
TLB miss: Page number 18 is not currently mapped to any frame.
TLB miss: Page number 251 is not currently mapped to any frame.
TLB miss: Page number 253 is not currently mapped to any frame.
TLB miss: Page number 245 is not currently mapped to any frame.
TLB miss: Page number 14 is not currently mapped to any frame.
TLB miss: Page number 196 is not currently mapped to any frame.
TLB miss: Page number 229 is not currently mapped to any frame.
TLB miss: Page number 9 is not currently mapped to any frame.
TLB miss: Page number 90 is not currently mapped to any frame.
TLB miss: Page number 222 is not currently mapped to any frame.
TLB miss: Page number 218 is not currently mapped to any frame.
TLB miss: Page number 42 is not currently mapped to any frame.
TLB miss: Page number 248 is not currently mapped to any frame.
TLB miss: Page number 5 is not currently mapped to any frame.
TLB miss: Page number 220 is not currently mapped to any frame.
TLB miss: Page number 87 is not currently mapped to any frame.
TLB miss: Page number 230 is not currently mapped to any frame.
TLB miss: Page number 126 is not currently mapped to any frame.
TLB miss: Page number 115 is not currently mapped to any frame.
TLB miss: Page number 26 is not currently mapped to any frame.
TLB miss: Page number 226 is not currently mapped to any frame.
TLB miss: Page number 71 is not currently mapped to any frame.
TLB miss: Page number 24 is not currently mapped to any frame.
TLB miss: Page number 234 is not currently mapped to any frame.
TLB miss: Page number 46 is not currently mapped to any frame.
TLB miss: Page number 97 is not currently mapped to any frame.
TLB miss: Page number 13 is not currently mapped to any frame.
TLB miss: Page number 11 is not currently mapped to any frame.
TLB miss: Page number 204 is not currently mapped to any frame.
TLB miss: Page number 205 is not currently mapped to any frame.
TLB miss: Page number 83 is not currently mapped to any frame.
TLB miss: Page number 125 is not currently mapped to any frame.
TLB miss: Page number 187 is not currently mapped to any frame.
TLB miss: Page number 69 is not currently mapped to any frame.
TLB miss: Page number 166 is not currently mapped to any frame.
TLB miss: Page number 64 is not currently mapped to any frame.
TLB miss: Page number 182 is not currently mapped to any frame.
TLB miss: Page number 14 is not currently mapped to any frame.
TLB miss: Page number 74 is not currently mapped to any frame.
TLB miss: Page number 15 is not currently mapped to any frame.
TLB miss: Page number 3 is not currently mapped to any frame.
TLB miss: Page number 229 is not currently mapped to any frame.
TLB miss: Page number 63 is not currently mapped to any frame.
TLB miss: Page number 48 is not currently mapped to any frame.
TLB miss: Page number 160 is not currently mapped to any frame.
TLB miss: Page number 64 is not currently mapped to any frame.
TLB miss: Page number 117 is not currently mapped to any frame.
TLB miss: Page number 146 is not currently mapped to any frame.
TLB miss: Page number 127 is not currently mapped to any frame.
TLB miss: Page number 184 is not currently mapped to any frame.
TLB miss: Page number 12 is not currently mapped to any frame.
TLB miss: Page number 17 is not currently mapped to any frame.
TLB miss: Page number 122 is not currently mapped to any frame.
TLB miss: Page number 130 is not currently mapped to any frame.
TLB miss: Page number 176 is not currently mapped to any frame.
TLB miss: Page number 37 is not currently mapped to any frame.
TLB miss: Page number 199 is not currently mapped to any frame.
TLB miss: Page number 88 is not currently mapped to any frame.
TLB miss: Page number 165 is not currently mapped to any frame.
TLB hit: Page number 69 is mapped to frame number 69.
TLB miss: Page number 101 is not currently mapped to any frame.
TLB miss: Page number 195 is not currently mapped to any frame.
TLB miss: Page number 235 is not currently mapped to any frame.
TLB miss: Page number 183 is not currently mapped to any frame.
TLB hit: Page number 83 is mapped to frame number 83.
TLB miss: Page number 60 is not currently mapped to any frame.
TLB miss: Page number 62 is not currently mapped to any frame.
TLB miss: Page number 54 is not currently mapped to any frame.
TLB miss: Page number 79 is not currently mapped to any frame.
TLB miss: Page number 5 is not currently mapped to any frame.
TLB hit: Page number 226 is mapped to frame number 226.
TLB miss: Page number 6 is not currently mapped to any frame.
TLB miss: Page number 155 is not currently mapped to any frame.
TLB miss: Page number 31 is not currently mapped to any frame.
TLB miss: Page number 215 is not currently mapped to any frame.
TLB miss: Page number 39 is not currently mapped to any frame.
TLB miss: Page number 57 is not currently mapped to any frame.
TLB hit: Page number 71 is mapped to frame number 71.
TLB miss: Page number 29 is not currently mapped to any frame.
TLB miss: Page number 153 is not currently mapped to any frame.
TLB miss: Page number 39 is not currently mapped to any frame.
TLB miss: Page number 191 is not currently mapped to any frame.
TLB miss: Page number 113 is not currently mapped to any frame.
TLB miss: Page number 91 is not currently mapped to any frame.
TLB miss: Page number 35 is not currently mapped to any frame.
TLB miss: Page number 136 is not currently mapped to any frame.
TLB miss: Page number 89 is not currently mapped to any frame.
TLB miss: Page number 43 is not currently mapped to any frame.
TLB miss: Page number 112 is not currently mapped to any frame.
TLB miss: Page number 162 is not currently mapped to any frame.
TLB miss: Page number 78 is not currently mapped to any frame.
TLB miss: Page number 76 is not currently mapped to any frame.
TLB hit: Page number 13 is mapped to frame number 13.
TLB miss: Page number 202 is not currently mapped to any frame.
TLB miss: Page number 80 is not currently mapped to any frame.
TLB miss: Page number 190 is not currently mapped to any frame.
TLB miss: Page number 252 is not currently mapped to any frame.
TLB miss: Page number 66 is not currently mapped to any frame.
TLB miss: Page number 231 is not currently mapped to any frame.
TLB miss: Page number 61 is not currently mapped to any frame.
TLB miss: Page number 247 is not currently mapped to any frame.
TLB hit: Page number 11 is mapped to frame number 11.
TLB miss: Page number 139 is not currently mapped to any frame.
TLB miss: Page number 80 is not currently mapped to any frame.
TLB miss: Page number 68 is not currently mapped to any frame.
TLB miss: Page number 38 is not currently mapped to any frame.
TLB miss: Page number 128 is not currently mapped to any frame.
TLB miss: Page number 113 is not currently mapped to any frame.
TLB miss: Page number 225 is not currently mapped to any frame.
TLB miss: Page number 77 is not currently mapped to any frame.
TLB miss: Page number 198 is not currently mapped to any frame.
TLB miss: Page number 227 is not currently mapped to any frame.
TLB miss: Page number 208 is not currently mapped to any frame.
TLB miss: Page number 9 is not currently mapped to any frame.
TLB miss: Page number 25 is not currently mapped to any frame.
TLB miss: Page number 98 is not currently mapped to any frame.
TLB miss: Page number 203 is not currently mapped to any frame.
TLB miss: Page number 211 is not currently mapped to any frame.
TLB miss: Page number 1 is not currently mapped to any frame.
TLB miss: Page number 50 is not currently mapped to any frame.
TLB hit: Page number 24 is mapped to frame number 24.
TLB miss: Page number 169 is not currently mapped to any frame.
TLB miss: Page number 246 is not currently mapped to any frame.
TLB miss: Page number 82 is not currently mapped to any frame.
TLB miss: Page number 182 is not currently mapped to any frame.
TLB miss: Page number 20 is not currently mapped to any frame.
TLB miss: Page number 248 is not currently mapped to any frame.
TLB miss: Page number 196 is not currently mapped to any frame.
TLB miss: Page number 96 is not currently mapped to any frame.
TLB miss: Page number 141 is not currently mapped to any frame.
TLB miss: Page number 143 is not currently mapped to any frame.
TLB miss: Page number 135 is not currently mapped to any frame.
TLB miss: Page number 160 is not currently mapped to any frame.
TLB miss: Page number 86 is not currently mapped to any frame.
TLB miss: Page number 51 is not currently mapped to any frame.
TLB miss: Page number 87 is not currently mapped to any frame.
TLB miss: Page number 236 is not currently mapped to any frame.
TLB miss: Page number 44 is not currently mapped to any frame.
TLB miss: Page number 40 is not currently mapped to any frame.
TLB miss: Page number 120 is not currently mapped to any frame.
TLB miss: Page number 138 is not currently mapped to any frame.
TLB miss: Page number 151 is not currently mapped to any frame.
TLB miss: Page number 110 is not currently mapped to any frame.
TLB hit: Page number 234 is mapped to frame number 234.
TLB miss: Page number 52 is not currently mapped to any frame.
TLB miss: Page number 16 is not currently mapped to any frame.
TLB miss: Page number 194 is not currently mapped to any frame.
TLB miss: Page number 104 is not currently mapped to any frame.
TLB miss: Page number 116 is not currently mapped to any frame.
TLB miss: Page number 217 is not currently mapped to any frame.
TLB miss: Page number 102 is not currently mapped to any frame.
TLB miss: Page number 124 is not currently mapped to any frame.
TLB miss: Page number 193 is not currently mapped to any frame.
TLB miss: Page number 243 is not currently mapped to any frame.
TLB miss: Page number 91 is not currently mapped to any frame.
TLB miss: Page number 157 is not currently mapped to any frame.
TLB miss: Page number 94 is not currently mapped to any frame.
TLB miss: Page number 27 is not currently mapped to any frame.
TLB miss: Page number 161 is not currently mapped to any frame.
TLB miss: Page number 15 is not currently mapped to any frame.
TLB miss: Page number 9 is not currently mapped to any frame.
TLB miss: Page number 147 is not currently mapped to any frame.
TLB miss: Page number 56 is not currently mapped to any frame.
TLB miss: Page number 142 is not currently mapped to any frame.
TLB miss: Page number 72 is not currently mapped to any frame.
TLB miss: Page number 92 is not currently mapped to any frame.
TLB miss: Page number 220 is not currently mapped to any frame.
TLB miss: Page number 93 is not currently mapped to any frame.
TLB miss: Page number 149 is not currently mapped to any frame.
TLB miss: Page number 119 is not currently mapped to any frame.
TLB miss: Page number 209 is not currently mapped to any frame.
TLB miss: Page number 194 is not currently mapped to any frame.
TLB miss: Page number 50 is not currently mapped to any frame.
TLB miss: Page number 158 is not currently mapped to any frame.
TLB miss: Page number 23 is not currently mapped to any frame.
TLB miss: Page number 240 is not currently mapped to any frame.
TLB miss: Page number 221 is not currently mapped to any frame.
TLB miss: Page number 22 is not currently mapped to any frame.
TLB miss: Page number 106 is not currently mapped to any frame.
TLB miss: Page number 179 is not currently mapped to any frame.
TLB miss: Page number 28 is not currently mapped to any frame.
TLB miss: Page number 224 is not currently mapped to any frame.
TLB miss: Page number 14 is not currently mapped to any frame.
TLB miss: Page number 131 is not currently mapped to any frame.
TLB miss: Page number 105 is not currently mapped to any frame.
TLB miss: Page number 250 is not currently mapped to any frame.
TLB miss: Page number 3 is not currently mapped to any frame.
TLB miss: Page number 163 is not currently mapped to any frame.
TLB miss: Page number 7 is not currently mapped to any frame.
TLB miss: Page number 33 is not currently mapped to any frame.
TLB miss: Page number 73 is not currently mapped to any frame.
TLB miss: Page number 21 is not currently mapped to any frame.
TLB miss: Page number 177 is not currently mapped to any frame.
TLB miss: Page number 222 is not currently mapped to any frame.
TLB miss: Page number 224 is not currently mapped to any frame.
TLB miss: Page number 216 is not currently mapped to any frame.
TLB miss: Page number 241 is not currently mapped to any frame.
TLB miss: Page number 99 is not currently mapped to any frame.
TLB miss: Page number 132 is not currently mapped to any frame.
TLB miss: Page number 168 is not currently mapped to any frame.
TLB miss: Page number 61 is not currently mapped to any frame.
TLB hit: Page number 125 is mapped to frame number 125.
TLB miss: Page number 121 is not currently mapped to any frame.
TLB miss: Page number 201 is not currently mapped to any frame.
TLB miss: Page number 219 is not currently mapped to any frame.
TLB miss: Page number 232 is not currently mapped to any frame.
TLB miss: Page number 191 is not currently mapped to any frame.
TLB miss: Page number 59 is not currently mapped to any frame.
TLB miss: Page number 133 is not currently mapped to any frame.
TLB hit: Page number 97 is mapped to frame number 97.
TLB miss: Page number 19 is not currently mapped to any frame.
TLB miss: Page number 185 is not currently mapped to any frame.
TLB miss: Page number 197 is not currently mapped to any frame.
TLB miss: Page number 42 is not currently mapped to any frame.
TLB miss: Page number 183 is not currently mapped to any frame.
TLB hit: Page number 205 is mapped to frame number 205.
TLB miss: Page number 18 is not currently mapped to any frame.
TLB miss: Page number 0 is not currently mapped to any frame.
TLB miss: Page number 172 is not currently mapped to any frame.
TLB miss: Page number 238 is not currently mapped to any frame.
TLB miss: Page number 107 is not currently mapped to any frame.
TLB miss: Page number 108 is not currently mapped to any frame.
TLB miss: Page number 242 is not currently mapped to any frame.
TLB miss: Page number 28 is not currently mapped to any frame.
TLB miss: Page number 90 is not currently mapped to any frame.
TLB miss: Page number 228 is not currently mapped to any frame.
TLB hit: Page number 69 is mapped to frame number 69.
TLB miss: Page number 223 is not currently mapped to any frame.
TLB miss: Page number 85 is not currently mapped to any frame.
TLB miss: Page number 173 is not currently mapped to any frame.
TLB miss: Page number 45 is not currently mapped to any frame.
TLB miss: Page number 174 is not currently mapped to any frame.
TLB miss: Page number 230 is not currently mapped to any frame.
TLB miss: Page number 200 is not currently mapped to any frame.
TLB miss: Page number 34 is not currently mapped to any frame.
TLB miss: Page number 207 is not currently mapped to any frame.
TLB miss: Page number 63 is not currently mapped to any frame.
TLB miss: Page number 239 is not currently mapped to any frame.
TLB miss: Page number 36 is not currently mapped to any frame.
TLB miss: Page number 65 is not currently mapped to any frame.
TLB hit: Page number 46 is mapped to frame number 46.
TLB miss: Page number 103 is not currently mapped to any frame.
TLB hit: Page number 187 is mapped to frame number 187.
TLB miss: Page number 4 is not currently mapped to any frame.
TLB miss: Page number 41 is not currently mapped to any frame.
TLB miss: Page number 49 is not currently mapped to any frame.
TLB miss: Page number 95 is not currently mapped to any frame.
TLB miss: Page number 212 is not currently mapped to any frame.
TLB miss: Page number 186 is not currently mapped to any frame.
TLB miss: Page number 75 is not currently mapped to any frame.
TLB miss: Page number 84 is not currently mapped to any frame.
TLB miss: Page number 244 is not currently mapped to any frame.
TLB miss: Page number 20 is not currently mapped to any frame.
TLB miss: Page number 114 is not currently mapped to any frame.
TLB miss: Page number 154 is not currently mapped to any frame.
TLB miss: Page number 102 is not currently mapped to any frame.
TLB miss: Page number 2 is not currently mapped to any frame.
TLB miss: Page number 235 is not currently mapped to any frame.
TLB miss: Page number 237 is not currently mapped to any frame.
TLB miss: Page number 229 is not currently mapped to any frame.
TLB miss: Page number 254 is not currently mapped to any frame.
TLB miss: Page number 180 is not currently mapped to any frame.
TLB miss: Page number 213 is not currently mapped to any frame.
TLB miss: Page number 249 is not currently mapped to any frame.
TLB miss: Page number 74 is not currently mapped to any frame.
TLB miss: Page number 206 is not currently mapped to any frame.
TLB miss: Page number 202 is not currently mapped to any frame.
TLB miss: Page number 26 is not currently mapped to any frame.
TLB miss: Page number 232 is not currently mapped to any frame.
TLB miss: Page number 246 is not currently mapped to any frame.
TLB hit: Page number 204 is mapped to frame number 204.
TLB miss: Page number 72 is not currently mapped to any frame.
TLB miss: Page number 214 is not currently mapped to any frame.
TLB miss: Page number 110 is not currently mapped to any frame.
TLB miss: Page number 100 is not currently mapped to any frame.
TLB miss: Page number 10 is not currently mapped to any frame.
TLB miss: Page number 210 is not currently mapped to any frame.
TLB miss: Page number 55 is not currently mapped to any frame.
TLB miss: Page number 8 is not currently mapped to any frame.
TLB miss: Page number 218 is not currently mapped to any frame.
TLB miss: Page number 31 is not currently mapped to any frame.
TLB miss: Page number 81 is not currently mapped to any frame.
TLB miss: Page number 253 is not currently mapped to any frame.
TLB miss: Page number 251 is not currently mapped to any frame.
TLB miss: Page number 188 is not currently mapped to any frame.
TLB miss: Page number 189 is not currently mapped to any frame.
TLB miss: Page number 67 is not currently mapped to any frame.
TLB miss: Page number 109 is not currently mapped to any frame.
TLB miss: Page number 171 is not currently mapped to any frame.
TLB miss: Page number 53 is not currently mapped to any frame.
TLB miss: Page number 150 is not currently mapped to any frame.
TLB miss: Page number 48 is not currently mapped to any frame.
TLB hit: Page number 166 is mapped to frame number 166.
TLB miss: Page number 254 is not currently mapped to any frame.
TLB miss: Page number 58 is not currently mapped to any frame.
TLB miss: Page number 255 is not currently mapped to any frame.
TLB miss: Page number 243 is not currently mapped to any frame.
TLB miss: Page number 213 is not currently mapped to any frame.
TLB miss: Page number 47 is not currently mapped to any frame.
TLB miss: Page number 32 is not currently mapped to any frame.
TLB miss: Page number 144 is not currently mapped to any frame.
TLB miss: Page number 64 is not currently mapped to any frame.
TLB miss: Page number 117 is not currently mapped to any frame.
TLB miss: Page number 146 is not currently mapped to any frame.
TLB miss: Page number 127 is not currently mapped to any frame.
TLB miss: Page number 184 is not currently mapped to any frame.
TLB miss: Page number 12 is not currently mapped to any frame.
TLB miss: Page number 17 is not currently mapped to any frame.
TLB miss: Page number 122 is not currently mapped to any frame.
TLB miss: Page number 130 is not currently mapped to any frame.
TLB miss: Page number 176 is not currently mapped to any frame.
TLB miss: Page number 37 is not currently mapped to any frame.
TLB miss: Page number 199 is not currently mapped to any frame.
TLB miss: Page number 88 is not currently mapped to any frame.
TLB miss: Page number 165 is not currently mapped to any frame.
TLB hit: Page number 69 is mapped to frame number 69.
TLB miss: Page number 101 is not currently mapped to any frame.
TLB miss: Page number 195 is not currently mapped to any frame.
TLB miss: Page number 235 is not currently mapped to any frame.
TLB miss: Page number 183 is not currently mapped to any frame.
TLB hit: Page number 83 is mapped to frame number 83.
TLB miss: Page number 60 is not currently mapped to any frame.
TLB miss: Page number 62 is not currently mapped to any frame.
TLB miss: Page number 54 is not currently mapped to any frame.
TLB miss: Page number 79 is not currently mapped to any frame.
TLB miss: Page number 5 is not currently mapped to any frame.
TLB hit: Page number 226 is mapped to frame number 226.
TLB miss: Page number 6 is not currently mapped to any frame.
TLB miss: Page number 155 is not currently mapped to any frame.
TLB miss: Page number 31 is not currently mapped to any frame.
TLB miss: Page number 215 is not currently mapped to any frame.
TLB miss: Page number 39 is not currently mapped to any frame.
TLB miss: Page number 57 is not currently mapped to any frame.
TLB hit: Page number 71 is mapped to frame number 71.
TLB miss: Page number 29 is not currently mapped to any frame.
TLB miss: Page number 153 is not currently mapped to any frame.
TLB miss: Page number 39 is not currently mapped to any frame.
TLB miss: Page number 191 is not currently mapped to any frame.
TLB miss: Page number 113 is not currently mapped to any frame.
TLB miss: Page number 91 is not currently mapped to any frame.
TLB miss: Page number 35 is not currently mapped to any frame.
TLB miss: Page number 136 is not currently mapped to any frame.
TLB miss: Page number 89 is not currently mapped to any frame.
TLB miss: Page number 43 is not currently mapped to any frame.
TLB miss: Page number 112 is not currently mapped to any frame.
TLB miss: Page number 162 is not currently mapped to any frame.
TLB miss: Page number 78 is not currently mapped to any frame.
TLB miss: Page number 76 is not currently mapped to any frame.
TLB hit: Page number 13 is mapped to frame number 13.
TLB miss: Page number 202 is not currently mapped to any frame.
TLB miss: Page number 80 is not currently mapped to any frame.
TLB miss: Page number 190 is not currently mapped to any frame.
TLB miss: Page number 252 is not currently mapped to any frame.
TLB miss: Page number 66 is not currently mapped to any frame.
TLB miss: Page number 231 is not currently mapped to any frame.
TLB miss: Page number 61 is not currently mapped to any frame.
TLB miss: Page number 247 is not currently mapped to any frame.
TLB hit: Page number 11 is mapped to frame number 11.
TLB miss: Page number 139 is not currently mapped to any frame.
TLB miss: Page number 80 is not currently mapped to any frame.
TLB miss: Page number 68 is not currently mapped to any frame.
TLB miss: Page number 38 is not currently mapped to any frame.
TLB miss: Page number 128 is not currently mapped to any frame.
TLB miss: Page number 113 is not currently mapped to any frame.
TLB miss: Page number 225 is not currently mapped to any frame.
TLB miss: Page number 77 is not currently mapped to any frame.
TLB miss: Page number 198 is not currently mapped to any frame.
TLB miss: Page number 227 is not currently mapped to any frame.
TLB miss: Page number 208 is not currently mapped to any frame.
TLB miss: Page number 9 is not currently mapped to any frame.
TLB miss: Page number 25 is not currently mapped to any frame.
TLB miss: Page number 98 is not currently mapped to any frame.
TLB miss: Page number 203 is not currently mapped to any frame.
TLB miss: Page number 211 is not currently mapped to any frame.
TLB miss: Page number 1 is not currently mapped to any frame.
TLB miss: Page number 50 is not currently mapped to any frame.
TLB hit: Page number 24 is mapped to frame number 24.
TLB miss: Page number 169 is not currently mapped to any frame.
TLB miss: Page number 246 is not currently mapped to any frame.
TLB miss: Page number 82 is not currently mapped to any frame.
TLB miss: Page number 182 is not currently mapped to any frame.
TLB miss: Page number 20 is not currently mapped to any frame.
TLB miss: Page number 248 is not currently mapped to any frame.
TLB miss: Page number 196 is not currently mapped to any frame.
TLB miss: Page number 96 is not currently mapped to any frame.
TLB miss: Page number 141 is not currently mapped to any frame.
TLB miss: Page number 143 is not currently mapped to any frame.
TLB miss: Page number 135 is not currently mapped to any frame.
TLB miss: Page number 160 is not currently mapped to any frame.
TLB miss: Page number 86 is not currently mapped to any frame.
TLB miss: Page number 51 is not currently mapped to any frame.
TLB miss: Page number 87 is not currently mapped to any frame.
TLB miss: Page number 236 is not currently mapped to any frame.
TLB miss: Page number 44 is not currently mapped to any frame.
TLB miss: Page number 40 is not currently mapped to any frame.
TLB miss: Page number 120 is not currently mapped to any frame.
TLB miss: Page number 138 is not currently mapped to any frame.
TLB miss: Page number 151 is not currently mapped to any frame.
TLB miss: Page number 110 is not currently mapped to any frame.
TLB hit: Page number 234 is mapped to frame number 234.
TLB miss: Page number 52 is not currently mapped to any frame.
TLB miss: Page number 16 is not currently mapped to any frame.
TLB miss: Page number 194 is not currently mapped to any frame.
TLB miss: Page number 104 is not currently mapped to any frame.
TLB miss: Page number 116 is not currently mapped to any frame.
TLB miss: Page number 217 is not currently mapped to any frame.
TLB miss: Page number 102 is not currently mapped to any frame.
TLB miss: Page number 124 is not currently mapped to any frame.
TLB miss: Page number 193 is not currently mapped to any frame.
TLB miss: Page number 243 is not currently mapped to any frame.
TLB miss: Page number 91 is not currently mapped to any frame.
TLB miss: Page number 157 is not currently mapped to any frame.
TLB miss: Page number 94 is not currently mapped to any frame.
TLB miss: Page number 27 is not currently mapped to any frame.
TLB miss: Page number 161 is not currently mapped to any frame.
TLB miss: Page number 15 is not currently mapped to any frame.
TLB miss: Page number 9 is not currently mapped to any frame.
TLB miss: Page number 147 is not currently mapped to any frame.
TLB miss: Page number 56 is not currently mapped to any frame.
TLB miss: Page number 142 is not currently mapped to any frame.
TLB miss: Page number 72 is not currently mapped to any frame.
TLB miss: Page number 92 is not currently mapped to any frame.
TLB miss: Page number 220 is not currently mapped to any frame.
TLB miss: Page number 93 is not currently mapped to any frame.
TLB miss: Page number 149 is not currently mapped to any frame.
TLB miss: Page number 119 is not currently mapped to any frame.
TLB miss: Page number 209 is not currently mapped to any frame.
TLB miss: Page number 194 is not currently mapped to any frame.
TLB miss: Page number 50 is not currently mapped to any frame.
TLB miss: Page number 158 is not currently mapped to any frame.
TLB miss: Page number 23 is not currently mapped to any frame.
TLB miss: Page number 240 is not currently mapped to any frame.
TLB miss: Page number 221 is not currently mapped to any frame.
TLB miss: Page number 22 is not currently mapped to any frame.
TLB miss: Page number 106 is not currently mapped to any frame.
TLB miss: Page number 179 is not currently mapped to any frame.
TLB miss: Page number 28 is not currently mapped to any frame.
TLB miss: Page number 224 is not currently mapped to any frame.
TLB miss: Page number 14 is not currently mapped to any frame.
TLB miss: Page number 131 is not currently mapped to any frame.
TLB miss: Page number 105 is not currently mapped to any frame.
TLB miss: Page number 250 is not currently mapped to any frame.
TLB miss: Page number 3 is not currently mapped to any frame.
TLB miss: Page number 163 is not currently mapped to any frame.
TLB miss: Page number 7 is not currently mapped to any frame.
TLB miss: Page number 33 is not currently mapped to any frame.
TLB miss: Page number 73 is not currently mapped to any frame.
TLB miss: Page number 21 is not currently mapped to any frame.
TLB miss: Page number 177 is not currently mapped to any frame.
TLB miss: Page number 222 is not currently mapped to any frame.
TLB miss: Page number 224 is not currently mapped to any frame.
TLB miss: Page number 216 is not currently mapped to any frame.
TLB miss: Page number 241 is not currently mapped to any frame.
TLB miss: Page number 99 is not currently mapped to any frame.
TLB miss: Page number 132 is not currently mapped to any frame.
TLB miss: Page number 168 is not currently mapped to any frame.
TLB miss: Page number 61 is not currently mapped to any frame.
TLB hit: Page number 125 is mapped to frame number 125.
TLB miss: Page number 121 is not currently mapped to any frame.
TLB miss: Page number 201 is not currently mapped to any frame.
TLB miss: Page number 219 is not currently mapped to any frame.
TLB miss: Page number 232 is not currently mapped to any frame.
TLB miss: Page number 191 is not currently mapped to any frame.
TLB miss: Page number 59 is not currently mapped to any frame.
TLB miss: Page number 133 is not currently mapped to any frame.
TLB hit: Page number 97 is mapped to frame number 97.
TLB miss: Page number 19 is not currently mapped to any frame.
TLB miss: Page number 185 is not currently mapped to any frame.
TLB miss: Page number 197 is not currently mapped to any frame.
TLB miss: Page number 42 is not currently mapped to any frame.
TLB miss: Page number 183 is not currently mapped to any frame.
TLB hit: Page number 205 is mapped to frame number 205.
TLB miss: Page number 18 is not currently mapped to any frame.
TLB miss: Page number 0 is not currently mapped to any frame.
TLB miss: Page number 172 is not currently mapped to any frame.
TLB miss: Page number 238 is not currently mapped to any frame.
TLB miss: Page number 107 is not currently mapped to any frame.
TLB miss: Page number 108 is not currently mapped to any frame.
TLB miss: Page number 242 is not currently mapped to any frame.
TLB miss: Page number 28 is not currently mapped to any frame.
TLB miss: Page number 90 is not currently mapped to any frame.
TLB miss: Page number 228 is not currently mapped to any frame.
TLB hit: Page number 69 is mapped to frame number 69.
TLB miss: Page number 223 is not currently mapped to any frame.
TLB miss: Page number 85 is not currently mapped to any frame.
TLB miss: Page number 173 is not currently mapped to any frame.
TLB miss: Page number 45 is not currently mapped to any frame.
TLB miss: Page number 174 is not currently mapped to any frame.
TLB miss: Page number 230 is not currently mapped to any frame.
TLB miss: Page number 200 is not currently mapped to any frame.
TLB miss: Page number 34 is not currently mapped to any frame.
TLB miss: Page number 207 is not currently mapped to any frame.
TLB miss: Page number 63 is not currently mapped to any frame.
TLB miss: Page number 239 is not currently mapped to any frame.
TLB miss: Page number 36 is not currently mapped to any frame.
TLB miss: Page number 65 is not currently mapped to any frame.
TLB hit: Page number 46 is mapped to frame number 46.
TLB miss: Page number 103 is not currently mapped to any frame.
TLB hit: Page number 187 is mapped to frame number 187.
TLB miss: Page number 4 is not currently mapped to any frame.
TLB miss: Page number 41 is not currently mapped to any frame.
TLB miss: Page number 49 is not currently mapped to any frame.
TLB miss: Page number 95 is not currently mapped to any frame.
TLB miss: Page number 212 is not currently mapped to any frame.
TLB miss: Page number 186 is not currently mapped to any frame.
TLB miss: Page number 75 is not currently mapped to any frame.
TLB miss: Page number 84 is not currently mapped to any frame.
TLB miss: Page number 244 is not currently mapped to any frame.
TLB miss: Page number 20 is not currently mapped to any frame.
TLB miss: Page number 114 is not currently mapped to any frame.
TLB miss: Page number 154 is not currently mapped to any frame.
TLB miss: Page number 102 is not currently mapped to any frame.
TLB miss: Page number 2 is not currently mapped to any frame.
TLB miss: Page number 235 is not currently mapped to any frame.
TLB miss: Page number 237 is not currently mapped to any frame.
TLB miss: Page number 229 is not currently mapped to any frame.
TLB miss: Page number 254 is not currently mapped to any frame.
TLB miss: Page number 180 is not currently mapped to any frame.
TLB miss: Page number 213 is not currently mapped to any frame.
TLB miss: Page number 249 is not currently mapped to any frame.
TLB miss: Page number 74 is not currently mapped to any frame.
TLB miss: Page number 206 is not currently mapped to any frame.
TLB miss: Page number 202 is not currently mapped to any frame.
TLB miss: Page number 26 is not currently mapped to any frame.
TLB miss: Page number 232 is not currently mapped to any frame.
TLB miss: Page number 246 is not currently mapped to any frame.
TLB hit: Page number 204 is mapped to frame number 204.
TLB miss: Page number 72 is not currently mapped to any frame.
TLB miss: Page number 214 is not currently mapped to any frame.
TLB miss: Page number 110 is not currently mapped to any frame.
Success
```

For output, check the output.txt:
```sh
Logical Address: 28241, Physical Address: 28241, Value: 255
Logical Address: 55012, Physical Address: 55012, Value: 255
Logical Address: 18441, Physical Address: 18441, Value: 255
Logical Address: 52477, Physical Address: 52477, Value: 255
Logical Address: 62977, Physical Address: 62977, Value: 255
Logical Address: 59477, Physical Address: 59477, Value: 255
Logical Address: 6711, Physical Address: 6711, Value: 255
Logical Address: 51946, Physical Address: 51946, Value: 255
Logical Address: 52911, Physical Address: 52911, Value: 255
Logical Address: 19141, Physical Address: 19141, Value: 255
Logical Address: 63847, Physical Address: 63847, Value: 255
Logical Address: 54746, Physical Address: 54746, Value: 255
Logical Address: 46176, Physical Address: 46176, Value: 255
Logical Address: 65077, Physical Address: 65077, Value: 255
Logical Address: 58777, Physical Address: 58777, Value: 255
Logical Address: 60877, Physical Address: 60877, Value: 255
Logical Address: 60177, Physical Address: 60177, Value: 255
Logical Address: 676, Physical Address: 676, Value: 255
Logical Address: 26311, Physical Address: 26311, Value: 255
Logical Address: 39611, Physical Address: 39611, Value: 255
Logical Address: 29376, Physical Address: 29376, Value: 255
Logical Address: 5141, Physical Address: 5141, Value: 255
Logical Address: 62712, Physical Address: 62712, Value: 255
Logical Address: 21676, Physical Address: 21676, Value: 255
Logical Address: 19311, Physical Address: 19311, Value: 255
Logical Address: 47746, Physical Address: 47746, Value: 255
Logical Address: 54311, Physical Address: 54311, Value: 255
Logical Address: 24476, Physical Address: 24476, Value: 255
Logical Address: 12576, Physical Address: 12576, Value: 255
Logical Address: 10741, Physical Address: 10741, Value: 255
Logical Address: 1111, Physical Address: 1111, Value: 255
Logical Address: 48011, Physical Address: 48011, Value: 255
Logical Address: 26576, Physical Address: 26576, Value: 255
Logical Address: 11876, Physical Address: 11876, Value: 255
Logical Address: 16776, Physical Address: 16776, Value: 255
Logical Address: 9341, Physical Address: 9341, Value: 255
Logical Address: 61312, Physical Address: 61312, Value: 255
Logical Address: 16341, Physical Address: 16341, Value: 255
Logical Address: 53177, Physical Address: 53177, Value: 255
Logical Address: 8811, Physical Address: 8811, Value: 255
Logical Address: 51246, Physical Address: 51246, Value: 255
Logical Address: 58946, Physical Address: 58946, Value: 255
Logical Address: 44776, Physical Address: 44776, Value: 255
Logical Address: 11611, Physical Address: 11611, Value: 255
Logical Address: 44511, Physical Address: 44511, Value: 255
Logical Address: 21941, Physical Address: 21941, Value: 255
Logical Address: 57112, Physical Address: 57112, Value: 255
Logical Address: 17741, Physical Address: 17741, Value: 255
Logical Address: 58512, Physical Address: 58512, Value: 255
Logical Address: 23076, Physical Address: 23076, Value: 255
Logical Address: 7241, Physical Address: 7241, Value: 255
Logical Address: 62012, Physical Address: 62012, Value: 255
Logical Address: 27711, Physical Address: 27711, Value: 255
Logical Address: 27541, Physical Address: 27541, Value: 255
Logical Address: 61047, Physical Address: 61047, Value: 255
Logical Address: 44076, Physical Address: 44076, Value: 255
Logical Address: 241, Physical Address: 241, Value: 255
Logical Address: 4611, Physical Address: 4611, Value: 255
Logical Address: 52646, Physical Address: 52646, Value: 255
Logical Address: 46876, Physical Address: 46876, Value: 255
Logical Address: 10911, Physical Address: 10911, Value: 255
Logical Address: 50546, Physical Address: 50546, Value: 255
Logical Address: 47576, Physical Address: 47576, Value: 255
Logical Address: 4876, Physical Address: 4876, Value: 255
Logical Address: 24911, Physical Address: 24911, Value: 255
Logical Address: 34276, Physical Address: 34276, Value: 255
Logical Address: 15111, Physical Address: 15111, Value: 255
Logical Address: 49146, Physical Address: 49146, Value: 255
Logical Address: 59646, Physical Address: 59646, Value: 255
Logical Address: 56146, Physical Address: 56146, Value: 255
Logical Address: 51511, Physical Address: 51511, Value: 255
Logical Address: 31211, Physical Address: 31211, Value: 255
Logical Address: 32176, Physical Address: 32176, Value: 255
Logical Address: 15811, Physical Address: 15811, Value: 255
Logical Address: 43111, Physical Address: 43111, Value: 255
Logical Address: 34011, Physical Address: 34011, Value: 255
Logical Address: 25441, Physical Address: 25441, Value: 255
Logical Address: 61747, Physical Address: 61747, Value: 255
Logical Address: 55446, Physical Address: 55446, Value: 255
Logical Address: 57546, Physical Address: 57546, Value: 255
Logical Address: 56846, Physical Address: 56846, Value: 255
Logical Address: 45476, Physical Address: 45476, Value: 255
Logical Address: 5576, Physical Address: 5576, Value: 255
Logical Address: 18876, Physical Address: 18876, Value: 255
Logical Address: 8641, Physical Address: 8641, Value: 255
Logical Address: 1811, Physical Address: 1811, Value: 255
Logical Address: 41976, Physical Address: 41976, Value: 255
Logical Address: 941, Physical Address: 941, Value: 255
Logical Address: 64112, Physical Address: 64112, Value: 255
Logical Address: 27011, Physical Address: 27011, Value: 255
Logical Address: 33576, Physical Address: 33576, Value: 255
Logical Address: 3741, Physical Address: 3741, Value: 255
Logical Address: 57377, Physical Address: 57377, Value: 255
Logical Address: 7411, Physical Address: 7411, Value: 255
Logical Address: 45911, Physical Address: 45911, Value: 255
Logical Address: 27276, Physical Address: 27276, Value: 255
Logical Address: 5841, Physical Address: 5841, Value: 255
Logical Address: 56677, Physical Address: 56677, Value: 255
Logical Address: 61577, Physical Address: 61577, Value: 255
Logical Address: 6011, Physical Address: 6011, Value: 255
Logical Address: 40576, Physical Address: 40576, Value: 255
Logical Address: 13011, Physical Address: 13011, Value: 255
Logical Address: 49846, Physical Address: 49846, Value: 255
Logical Address: 53611, Physical Address: 53611, Value: 255
Logical Address: 30511, Physical Address: 30511, Value: 255
Logical Address: 38211, Physical Address: 38211, Value: 255
Logical Address: 24041, Physical Address: 24041, Value: 255
Logical Address: 56412, Physical Address: 56412, Value: 255
Logical Address: 23776, Physical Address: 23776, Value: 255
Logical Address: 18611, Physical Address: 18611, Value: 255
Logical Address: 36376, Physical Address: 36376, Value: 255
Logical Address: 14411, Physical Address: 14411, Value: 255
Logical Address: 37776, Physical Address: 37776, Value: 255
Logical Address: 2341, Physical Address: 2341, Value: 255
Logical Address: 3911, Physical Address: 3911, Value: 255
Logical Address: 41276, Physical Address: 41276, Value: 255
Logical Address: 6976, Physical Address: 6976, Value: 255
Logical Address: 24211, Physical Address: 24211, Value: 255
Logical Address: 40311, Physical Address: 40311, Value: 255
Logical Address: 23341, Physical Address: 23341, Value: 255
Logical Address: 62447, Physical Address: 62447, Value: 255
Logical Address: 49411, Physical Address: 49411, Value: 255
Logical Address: 31911, Physical Address: 31911, Value: 255
Logical Address: 26141, Physical Address: 26141, Value: 255
Logical Address: 55712, Physical Address: 55712, Value: 255
Logical Address: 29811, Physical Address: 29811, Value: 255
Logical Address: 26841, Physical Address: 26841, Value: 255
Logical Address: 49677, Physical Address: 49677, Value: 255
Logical Address: 4176, Physical Address: 4176, Value: 255
Logical Address: 13541, Physical Address: 13541, Value: 255
Logical Address: 59912, Physical Address: 59912, Value: 255
Logical Address: 28411, Physical Address: 28411, Value: 255
Logical Address: 38911, Physical Address: 38911, Value: 255
Logical Address: 35411, Physical Address: 35411, Value: 255
Logical Address: 30776, Physical Address: 30776, Value: 255
Logical Address: 10476, Physical Address: 10476, Value: 255
Logical Address: 11441, Physical Address: 11441, Value: 255
Logical Address: 60612, Physical Address: 60612, Value: 255
Logical Address: 22376, Physical Address: 22376, Value: 255
Logical Address: 13276, Physical Address: 13276, Value: 255
Logical Address: 22111, Physical Address: 22111, Value: 255
Logical Address: 41011, Physical Address: 41011, Value: 255
Logical Address: 34711, Physical Address: 34711, Value: 255
Logical Address: 36811, Physical Address: 36811, Value: 255
Logical Address: 36111, Physical Address: 36111, Value: 255
Logical Address: 24741, Physical Address: 24741, Value: 255
Logical Address: 50377, Physical Address: 50377, Value: 255
Logical Address: 63677, Physical Address: 63677, Value: 255
Logical Address: 5311, Physical Address: 5311, Value: 255
Logical Address: 46611, Physical Address: 46611, Value: 255
Logical Address: 21241, Physical Address: 21241, Value: 255
Logical Address: 63147, Physical Address: 63147, Value: 255
Logical Address: 43376, Physical Address: 43376, Value: 255
Logical Address: 6276, Physical Address: 6276, Value: 255
Logical Address: 12841, Physical Address: 12841, Value: 255
Logical Address: 411, Physical Address: 411, Value: 255
Logical Address: 54046, Physical Address: 54046, Value: 255
Logical Address: 52211, Physical Address: 52211, Value: 255
Logical Address: 25176, Physical Address: 25176, Value: 255
Logical Address: 6541, Physical Address: 6541, Value: 255
Logical Address: 2511, Physical Address: 2511, Value: 255
Logical Address: 53346, Physical Address: 53346, Value: 255
Logical Address: 58246, Physical Address: 58246, Value: 255
Logical Address: 50811, Physical Address: 50811, Value: 255
Logical Address: 19841, Physical Address: 19841, Value: 255
Logical Address: 57812, Physical Address: 57812, Value: 255
Logical Address: 29111, Physical Address: 29111, Value: 255
Logical Address: 32876, Physical Address: 32876, Value: 255
Logical Address: 9776, Physical Address: 9776, Value: 255
Logical Address: 17476, Physical Address: 17476, Value: 255
Logical Address: 20711, Physical Address: 20711, Value: 255
Logical Address: 35676, Physical Address: 35676, Value: 255
Logical Address: 3041, Physical Address: 3041, Value: 255
Logical Address: 63412, Physical Address: 63412, Value: 255
Logical Address: 15641, Physical Address: 15641, Value: 255
Logical Address: 59212, Physical Address: 59212, Value: 255
Logical Address: 17041, Physical Address: 17041, Value: 255
Logical Address: 64547, Physical Address: 64547, Value: 255
Logical Address: 48711, Physical Address: 48711, Value: 255
Logical Address: 20541, Physical Address: 20541, Value: 255
Logical Address: 51777, Physical Address: 51777, Value: 255
Logical Address: 3476, Physical Address: 3476, Value: 255
Logical Address: 19576, Physical Address: 19576, Value: 255
Logical Address: 20011, Physical Address: 20011, Value: 255
Logical Address: 41711, Physical Address: 41711, Value: 255
Logical Address: 28676, Physical Address: 28676, Value: 255
Logical Address: 11176, Physical Address: 11176, Value: 255
Logical Address: 22811, Physical Address: 22811, Value: 255
Logical Address: 34976, Physical Address: 34976, Value: 255
Logical Address: 9076, Physical Address: 9076, Value: 255
Logical Address: 23511, Physical Address: 23511, Value: 255
Logical Address: 28941, Physical Address: 28941, Value: 255
Logical Address: 48977, Physical Address: 48977, Value: 255
Logical Address: 10211, Physical Address: 10211, Value: 255
Logical Address: 39176, Physical Address: 39176, Value: 255
Logical Address: 7676, Physical Address: 7676, Value: 255
Logical Address: 18176, Physical Address: 18176, Value: 255
Logical Address: 14676, Physical Address: 14676, Value: 255
Logical Address: 10041, Physical Address: 10041, Value: 255
Logical Address: 55277, Physical Address: 55277, Value: 255
Logical Address: 8111, Physical Address: 8111, Value: 255
Logical Address: 39876, Physical Address: 39876, Value: 255
Logical Address: 1641, Physical Address: 1641, Value: 255
Logical Address: 58077, Physical Address: 58077, Value: 255
Logical Address: 1376, Physical Address: 1376, Value: 255
Logical Address: 20276, Physical Address: 20276, Value: 255
Logical Address: 13976, Physical Address: 13976, Value: 255
Logical Address: 16076, Physical Address: 16076, Value: 255
Logical Address: 15376, Physical Address: 15376, Value: 255
Logical Address: 21411, Physical Address: 21411, Value: 255
Logical Address: 47046, Physical Address: 47046, Value: 255
Logical Address: 60346, Physical Address: 60346, Value: 255
Logical Address: 50111, Physical Address: 50111, Value: 255
Logical Address: 25876, Physical Address: 25876, Value: 255
Logical Address: 17911, Physical Address: 17911, Value: 255
Logical Address: 42411, Physical Address: 42411, Value: 255
Logical Address: 22641, Physical Address: 22641, Value: 255
Logical Address: 51077, Physical Address: 51077, Value: 255
Logical Address: 9511, Physical Address: 9511, Value: 255
Logical Address: 45211, Physical Address: 45211, Value: 255
Logical Address: 33311, Physical Address: 33311, Value: 255
Logical Address: 31476, Physical Address: 31476, Value: 255
Logical Address: 4441, Physical Address: 4441, Value: 255
Logical Address: 3211, Physical Address: 3211, Value: 255
Logical Address: 47311, Physical Address: 47311, Value: 255
Logical Address: 32611, Physical Address: 32611, Value: 255
Logical Address: 37511, Physical Address: 37511, Value: 255
Logical Address: 30076, Physical Address: 30076, Value: 255
Logical Address: 16511, Physical Address: 16511, Value: 255
Logical Address: 37076, Physical Address: 37076, Value: 255
Logical Address: 8376, Physical Address: 8376, Value: 255
Logical Address: 12141, Physical Address: 12141, Value: 255
Logical Address: 54577, Physical Address: 54577, Value: 255
Logical Address: 62277, Physical Address: 62277, Value: 255
Logical Address: 65512, Physical Address: 65512, Value: 255
Logical Address: 14941, Physical Address: 14941, Value: 255
Logical Address: 65247, Physical Address: 65247, Value: 255
Logical Address: 42676, Physical Address: 42676, Value: 255
Logical Address: 12311, Physical Address: 12311, Value: 255
Logical Address: 38476, Physical Address: 38476, Value: 255
Logical Address: 13711, Physical Address: 13711, Value: 255
Logical Address: 43811, Physical Address: 43811, Value: 255
Logical Address: 27976, Physical Address: 27976, Value: 255
Logical Address: 17211, Physical Address: 17211, Value: 255
Logical Address: 48446, Physical Address: 48446, Value: 255
Logical Address: 48276, Physical Address: 48276, Value: 255
Logical Address: 64377, Physical Address: 64377, Value: 255
Logical Address: 64812, Physical Address: 64812, Value: 255
Logical Address: 20976, Physical Address: 20976, Value: 255
Logical Address: 7941, Physical Address: 7941, Value: 255
Logical Address: 55977, Physical Address: 55977, Value: 255
Logical Address: 2076, Physical Address: 2076, Value: 255
Logical Address: 14241, Physical Address: 14241, Value: 255
Logical Address: 53877, Physical Address: 53877, Value: 255
Logical Address: 2776, Physical Address: 2776, Value: 255
Logical Address: 25611, Physical Address: 25611, Value: 255
Logical Address: 28241, Physical Address: 28241, Value: 255
Logical Address: 55012, Physical Address: 55012, Value: 255
Logical Address: 18441, Physical Address: 18441, Value: 255
Logical Address: 52477, Physical Address: 52477, Value: 255
Logical Address: 62977, Physical Address: 62977, Value: 255
Logical Address: 59477, Physical Address: 59477, Value: 255
Logical Address: 6711, Physical Address: 6711, Value: 255
Logical Address: 51946, Physical Address: 51946, Value: 255
Logical Address: 52911, Physical Address: 52911, Value: 255
Logical Address: 19141, Physical Address: 19141, Value: 255
Logical Address: 63847, Physical Address: 63847, Value: 255
Logical Address: 54746, Physical Address: 54746, Value: 255
Logical Address: 46176, Physical Address: 46176, Value: 255
Logical Address: 65077, Physical Address: 65077, Value: 255
Logical Address: 58777, Physical Address: 58777, Value: 255
Logical Address: 60877, Physical Address: 60877, Value: 255
Logical Address: 60177, Physical Address: 60177, Value: 255
Logical Address: 676, Physical Address: 676, Value: 255
Logical Address: 26311, Physical Address: 26311, Value: 255
Logical Address: 39611, Physical Address: 39611, Value: 255
Logical Address: 29376, Physical Address: 29376, Value: 255
Logical Address: 5141, Physical Address: 5141, Value: 255
Logical Address: 62712, Physical Address: 62712, Value: 255
Logical Address: 21676, Physical Address: 21676, Value: 255
Logical Address: 19311, Physical Address: 19311, Value: 255
Logical Address: 47746, Physical Address: 47746, Value: 255
Logical Address: 54311, Physical Address: 54311, Value: 255
Logical Address: 24476, Physical Address: 24476, Value: 255
Logical Address: 12576, Physical Address: 12576, Value: 255
Logical Address: 10741, Physical Address: 10741, Value: 255
Logical Address: 1111, Physical Address: 1111, Value: 255
Logical Address: 48011, Physical Address: 48011, Value: 255
Logical Address: 26576, Physical Address: 26576, Value: 255
Logical Address: 11876, Physical Address: 11876, Value: 255
Logical Address: 16776, Physical Address: 16776, Value: 255
Logical Address: 9341, Physical Address: 9341, Value: 255
Logical Address: 61312, Physical Address: 61312, Value: 255
Logical Address: 16341, Physical Address: 16341, Value: 255
Logical Address: 53177, Physical Address: 53177, Value: 255
Logical Address: 8811, Physical Address: 8811, Value: 255
Logical Address: 51246, Physical Address: 51246, Value: 255
Logical Address: 58946, Physical Address: 58946, Value: 255
Logical Address: 44776, Physical Address: 44776, Value: 255
Logical Address: 11611, Physical Address: 11611, Value: 255
Logical Address: 44511, Physical Address: 44511, Value: 255
Logical Address: 21941, Physical Address: 21941, Value: 255
Logical Address: 57112, Physical Address: 57112, Value: 255
Logical Address: 17741, Physical Address: 17741, Value: 255
Logical Address: 58512, Physical Address: 58512, Value: 255
Logical Address: 23076, Physical Address: 23076, Value: 255
Logical Address: 7241, Physical Address: 7241, Value: 255
Logical Address: 62012, Physical Address: 62012, Value: 255
Logical Address: 27711, Physical Address: 27711, Value: 255
Logical Address: 27541, Physical Address: 27541, Value: 255
Logical Address: 61047, Physical Address: 61047, Value: 255
Logical Address: 44076, Physical Address: 44076, Value: 255
Logical Address: 241, Physical Address: 241, Value: 255
Logical Address: 4611, Physical Address: 4611, Value: 255
Logical Address: 52646, Physical Address: 52646, Value: 255
Logical Address: 46876, Physical Address: 46876, Value: 255
Logical Address: 10911, Physical Address: 10911, Value: 255
Logical Address: 50546, Physical Address: 50546, Value: 255
Logical Address: 47576, Physical Address: 47576, Value: 255
Logical Address: 4876, Physical Address: 4876, Value: 255
Logical Address: 24911, Physical Address: 24911, Value: 255
Logical Address: 34276, Physical Address: 34276, Value: 255
Logical Address: 15111, Physical Address: 15111, Value: 255
Logical Address: 49146, Physical Address: 49146, Value: 255
Logical Address: 59646, Physical Address: 59646, Value: 255
Logical Address: 56146, Physical Address: 56146, Value: 255
Logical Address: 51511, Physical Address: 51511, Value: 255
Logical Address: 31211, Physical Address: 31211, Value: 255
Logical Address: 32176, Physical Address: 32176, Value: 255
Logical Address: 15811, Physical Address: 15811, Value: 255
Logical Address: 43111, Physical Address: 43111, Value: 255
Logical Address: 34011, Physical Address: 34011, Value: 255
Logical Address: 25441, Physical Address: 25441, Value: 255
Logical Address: 61747, Physical Address: 61747, Value: 255
Logical Address: 55446, Physical Address: 55446, Value: 255
Logical Address: 57546, Physical Address: 57546, Value: 255
Logical Address: 56846, Physical Address: 56846, Value: 255
Logical Address: 45476, Physical Address: 45476, Value: 255
Logical Address: 5576, Physical Address: 5576, Value: 255
Logical Address: 18876, Physical Address: 18876, Value: 255
Logical Address: 8641, Physical Address: 8641, Value: 255
Logical Address: 1811, Physical Address: 1811, Value: 255
Logical Address: 41976, Physical Address: 41976, Value: 255
Logical Address: 941, Physical Address: 941, Value: 255
Logical Address: 64112, Physical Address: 64112, Value: 255
Logical Address: 27011, Physical Address: 27011, Value: 255
Logical Address: 33576, Physical Address: 33576, Value: 255
Logical Address: 3741, Physical Address: 3741, Value: 255
Logical Address: 57377, Physical Address: 57377, Value: 255
Logical Address: 7411, Physical Address: 7411, Value: 255
Logical Address: 45911, Physical Address: 45911, Value: 255
Logical Address: 27276, Physical Address: 27276, Value: 255
Logical Address: 5841, Physical Address: 5841, Value: 255
Logical Address: 56677, Physical Address: 56677, Value: 255
Logical Address: 61577, Physical Address: 61577, Value: 255
Logical Address: 6011, Physical Address: 6011, Value: 255
Logical Address: 40576, Physical Address: 40576, Value: 255
Logical Address: 13011, Physical Address: 13011, Value: 255
Logical Address: 49846, Physical Address: 49846, Value: 255
Logical Address: 53611, Physical Address: 53611, Value: 255
Logical Address: 30511, Physical Address: 30511, Value: 255
Logical Address: 38211, Physical Address: 38211, Value: 255
Logical Address: 24041, Physical Address: 24041, Value: 255
Logical Address: 56412, Physical Address: 56412, Value: 255
Logical Address: 23776, Physical Address: 23776, Value: 255
Logical Address: 18611, Physical Address: 18611, Value: 255
Logical Address: 36376, Physical Address: 36376, Value: 255
Logical Address: 14411, Physical Address: 14411, Value: 255
Logical Address: 37776, Physical Address: 37776, Value: 255
Logical Address: 2341, Physical Address: 2341, Value: 255
Logical Address: 3911, Physical Address: 3911, Value: 255
Logical Address: 41276, Physical Address: 41276, Value: 255
Logical Address: 6976, Physical Address: 6976, Value: 255
Logical Address: 24211, Physical Address: 24211, Value: 255
Logical Address: 40311, Physical Address: 40311, Value: 255
Logical Address: 23341, Physical Address: 23341, Value: 255
Logical Address: 62447, Physical Address: 62447, Value: 255
Logical Address: 49411, Physical Address: 49411, Value: 255
Logical Address: 31911, Physical Address: 31911, Value: 255
Logical Address: 26141, Physical Address: 26141, Value: 255
Logical Address: 55712, Physical Address: 55712, Value: 255
Logical Address: 29811, Physical Address: 29811, Value: 255
Logical Address: 26841, Physical Address: 26841, Value: 255
Logical Address: 49677, Physical Address: 49677, Value: 255
Logical Address: 4176, Physical Address: 4176, Value: 255
Logical Address: 13541, Physical Address: 13541, Value: 255
Logical Address: 59912, Physical Address: 59912, Value: 255
Logical Address: 28411, Physical Address: 28411, Value: 255
Logical Address: 38911, Physical Address: 38911, Value: 255
Logical Address: 35411, Physical Address: 35411, Value: 255
Logical Address: 30776, Physical Address: 30776, Value: 255
Logical Address: 10476, Physical Address: 10476, Value: 255
Logical Address: 11441, Physical Address: 11441, Value: 255
Logical Address: 60612, Physical Address: 60612, Value: 255
Logical Address: 22376, Physical Address: 22376, Value: 255
Logical Address: 13276, Physical Address: 13276, Value: 255
Logical Address: 22111, Physical Address: 22111, Value: 255
Logical Address: 41011, Physical Address: 41011, Value: 255
Logical Address: 34711, Physical Address: 34711, Value: 255
Logical Address: 36811, Physical Address: 36811, Value: 255
Logical Address: 36111, Physical Address: 36111, Value: 255
Logical Address: 24741, Physical Address: 24741, Value: 255
Logical Address: 50377, Physical Address: 50377, Value: 255
Logical Address: 63677, Physical Address: 63677, Value: 255
Logical Address: 5311, Physical Address: 5311, Value: 255
Logical Address: 46611, Physical Address: 46611, Value: 255
Logical Address: 21241, Physical Address: 21241, Value: 255
Logical Address: 63147, Physical Address: 63147, Value: 255
Logical Address: 43376, Physical Address: 43376, Value: 255
Logical Address: 6276, Physical Address: 6276, Value: 255
Logical Address: 12841, Physical Address: 12841, Value: 255
Logical Address: 411, Physical Address: 411, Value: 255
Logical Address: 54046, Physical Address: 54046, Value: 255
Logical Address: 52211, Physical Address: 52211, Value: 255
Logical Address: 25176, Physical Address: 25176, Value: 255
Logical Address: 6541, Physical Address: 6541, Value: 255
Logical Address: 2511, Physical Address: 2511, Value: 255
Logical Address: 53346, Physical Address: 53346, Value: 255
Logical Address: 58246, Physical Address: 58246, Value: 255
Logical Address: 50811, Physical Address: 50811, Value: 255
Logical Address: 19841, Physical Address: 19841, Value: 255
Logical Address: 57812, Physical Address: 57812, Value: 255
Logical Address: 29111, Physical Address: 29111, Value: 255
Logical Address: 32876, Physical Address: 32876, Value: 255
Logical Address: 9776, Physical Address: 9776, Value: 255
Logical Address: 17476, Physical Address: 17476, Value: 255
Logical Address: 20711, Physical Address: 20711, Value: 255
Logical Address: 35676, Physical Address: 35676, Value: 255
Logical Address: 3041, Physical Address: 3041, Value: 255
Logical Address: 63412, Physical Address: 63412, Value: 255
Logical Address: 15641, Physical Address: 15641, Value: 255
Logical Address: 59212, Physical Address: 59212, Value: 255
Logical Address: 17041, Physical Address: 17041, Value: 255
Logical Address: 64547, Physical Address: 64547, Value: 255
Logical Address: 48711, Physical Address: 48711, Value: 255
Logical Address: 20541, Physical Address: 20541, Value: 255
Logical Address: 51777, Physical Address: 51777, Value: 255
Logical Address: 3476, Physical Address: 3476, Value: 255
Logical Address: 19576, Physical Address: 19576, Value: 255
Logical Address: 20011, Physical Address: 20011, Value: 255
Logical Address: 41711, Physical Address: 41711, Value: 255
Logical Address: 28676, Physical Address: 28676, Value: 255
Logical Address: 11176, Physical Address: 11176, Value: 255
Logical Address: 22811, Physical Address: 22811, Value: 255
Logical Address: 34976, Physical Address: 34976, Value: 255
Logical Address: 9076, Physical Address: 9076, Value: 255
Logical Address: 23511, Physical Address: 23511, Value: 255
Logical Address: 28941, Physical Address: 28941, Value: 255
Logical Address: 48977, Physical Address: 48977, Value: 255
Logical Address: 10211, Physical Address: 10211, Value: 255
Logical Address: 39176, Physical Address: 39176, Value: 255
Logical Address: 7676, Physical Address: 7676, Value: 255
Logical Address: 18176, Physical Address: 18176, Value: 255
Logical Address: 14676, Physical Address: 14676, Value: 255
Logical Address: 10041, Physical Address: 10041, Value: 255
Logical Address: 55277, Physical Address: 55277, Value: 255
Logical Address: 8111, Physical Address: 8111, Value: 255
Logical Address: 39876, Physical Address: 39876, Value: 255
Logical Address: 1641, Physical Address: 1641, Value: 255
Logical Address: 58077, Physical Address: 58077, Value: 255
Logical Address: 1376, Physical Address: 1376, Value: 255
Logical Address: 20276, Physical Address: 20276, Value: 255
Logical Address: 13976, Physical Address: 13976, Value: 255
Logical Address: 16076, Physical Address: 16076, Value: 255
Logical Address: 15376, Physical Address: 15376, Value: 255
Logical Address: 21411, Physical Address: 21411, Value: 255
Logical Address: 47046, Physical Address: 47046, Value: 255
Logical Address: 60346, Physical Address: 60346, Value: 255
Logical Address: 50111, Physical Address: 50111, Value: 255
Logical Address: 25876, Physical Address: 25876, Value: 255
Logical Address: 17911, Physical Address: 17911, Value: 255
Logical Address: 42411, Physical Address: 42411, Value: 255
Logical Address: 22641, Physical Address: 22641, Value: 255
Logical Address: 51077, Physical Address: 51077, Value: 255
Logical Address: 9511, Physical Address: 9511, Value: 255
Logical Address: 45211, Physical Address: 45211, Value: 255
Logical Address: 33311, Physical Address: 33311, Value: 255
Logical Address: 31476, Physical Address: 31476, Value: 255
Logical Address: 4441, Physical Address: 4441, Value: 255
Logical Address: 3211, Physical Address: 3211, Value: 255
Logical Address: 47311, Physical Address: 47311, Value: 255
Logical Address: 32611, Physical Address: 32611, Value: 255
Logical Address: 37511, Physical Address: 37511, Value: 255
Logical Address: 30076, Physical Address: 30076, Value: 255
Logical Address: 16511, Physical Address: 16511, Value: 255
Logical Address: 41160, Physical Address: 41160, Value: 255
Logical Address: 12460, Physical Address: 12460, Value: 255
Logical Address: 16225, Physical Address: 16225, Value: 255
Logical Address: 58661, Physical Address: 58661, Value: 255
Logical Address: 825, Physical Address: 825, Value: 255
Logical Address: 4060, Physical Address: 4060, Value: 255
Logical Address: 19025, Physical Address: 19025, Value: 255
Logical Address: 3795, Physical Address: 3795, Value: 255
Logical Address: 46760, Physical Address: 46760, Value: 255
Logical Address: 16395, Physical Address: 16395, Value: 255
Logical Address: 42560, Physical Address: 42560, Value: 255
Logical Address: 17795, Physical Address: 17795, Value: 255
Logical Address: 47895, Physical Address: 47895, Value: 255
Logical Address: 32060, Physical Address: 32060, Value: 255
Logical Address: 21295, Physical Address: 21295, Value: 255
Logical Address: 52530, Physical Address: 52530, Value: 255
Logical Address: 52360, Physical Address: 52360, Value: 255
Logical Address: 2925, Physical Address: 2925, Value: 255
Logical Address: 3360, Physical Address: 3360, Value: 255
Logical Address: 25060, Physical Address: 25060, Value: 255
Logical Address: 12025, Physical Address: 12025, Value: 255
Logical Address: 60061, Physical Address: 60061, Value: 255
Logical Address: 6160, Physical Address: 6160, Value: 255
Logical Address: 18325, Physical Address: 18325, Value: 255
Logical Address: 57961, Physical Address: 57961, Value: 255
Logical Address: 6860, Physical Address: 6860, Value: 255
Logical Address: 29695, Physical Address: 29695, Value: 255
Logical Address: 32325, Physical Address: 32325, Value: 255
Logical Address: 59096, Physical Address: 59096, Value: 255
Logical Address: 22525, Physical Address: 22525, Value: 255
Logical Address: 56561, Physical Address: 56561, Value: 255
Logical Address: 1525, Physical Address: 1525, Value: 255
Logical Address: 63561, Physical Address: 63561, Value: 255
Logical Address: 10795, Physical Address: 10795, Value: 255
Logical Address: 56030, Physical Address: 56030, Value: 255
Logical Address: 56995, Physical Address: 56995, Value: 255
Logical Address: 23225, Physical Address: 23225, Value: 255
Logical Address: 2395, Physical Address: 2395, Value: 255
Logical Address: 58830, Physical Address: 58830, Value: 255
Logical Address: 50260, Physical Address: 50260, Value: 255
Logical Address: 3625, Physical Address: 3625, Value: 255
Logical Address: 62861, Physical Address: 62861, Value: 255
Logical Address: 64961, Physical Address: 64961, Value: 255
Logical Address: 64261, Physical Address: 64261, Value: 255
Logical Address: 4760, Physical Address: 4760, Value: 255
Logical Address: 30395, Physical Address: 30395, Value: 255
Logical Address: 43695, Physical Address: 43695, Value: 255
Logical Address: 33460, Physical Address: 33460, Value: 255
Logical Address: 9225, Physical Address: 9225, Value: 255
Logical Address: 1260, Physical Address: 1260, Value: 255
Logical Address: 25760, Physical Address: 25760, Value: 255
Logical Address: 23395, Physical Address: 23395, Value: 255
Logical Address: 51830, Physical Address: 51830, Value: 255
Logical Address: 58395, Physical Address: 58395, Value: 255
Logical Address: 28560, Physical Address: 28560, Value: 255
Logical Address: 16660, Physical Address: 16660, Value: 255
Logical Address: 14825, Physical Address: 14825, Value: 255
Logical Address: 5195, Physical Address: 5195, Value: 255
Logical Address: 52095, Physical Address: 52095, Value: 255
Logical Address: 30660, Physical Address: 30660, Value: 255
Logical Address: 15960, Physical Address: 15960, Value: 255
Logical Address: 20860, Physical Address: 20860, Value: 255
Logical Address: 13425, Physical Address: 13425, Value: 255
Logical Address: 65396, Physical Address: 65396, Value: 255
Logical Address: 20425, Physical Address: 20425, Value: 255
Logical Address: 57261, Physical Address: 57261, Value: 255
Logical Address: 12895, Physical Address: 12895, Value: 255
Logical Address: 55330, Physical Address: 55330, Value: 255
Logical Address: 63030, Physical Address: 63030, Value: 255
Logical Address: 48860, Physical Address: 48860, Value: 255
Logical Address: 15695, Physical Address: 15695, Value: 255
Logical Address: 48595, Physical Address: 48595, Value: 255
Logical Address: 26025, Physical Address: 26025, Value: 255
Logical Address: 61196, Physical Address: 61196, Value: 255
Logical Address: 21825, Physical Address: 21825, Value: 255
Logical Address: 62596, Physical Address: 62596, Value: 255
Logical Address: 27160, Physical Address: 27160, Value: 255
Logical Address: 11325, Physical Address: 11325, Value: 255
Logical Address: 560, Physical Address: 560, Value: 255
Logical Address: 31795, Physical Address: 31795, Value: 255
Logical Address: 31625, Physical Address: 31625, Value: 255
Logical Address: 65131, Physical Address: 65131, Value: 255
Logical Address: 48160, Physical Address: 48160, Value: 255
Logical Address: 4325, Physical Address: 4325, Value: 255
Logical Address: 8695, Physical Address: 8695, Value: 255
Logical Address: 56730, Physical Address: 56730, Value: 255
Logical Address: 50960, Physical Address: 50960, Value: 255
Logical Address: 14995, Physical Address: 14995, Value: 255
Logical Address: 54630, Physical Address: 54630, Value: 255
Logical Address: 51660, Physical Address: 51660, Value: 255
Logical Address: 8960, Physical Address: 8960, Value: 255
Logical Address: 28995, Physical Address: 28995, Value: 255
Logical Address: 38360, Physical Address: 38360, Value: 255
Logical Address: 19195, Physical Address: 19195, Value: 255
Logical Address: 53230, Physical Address: 53230, Value: 255
Logical Address: 63730, Physical Address: 63730, Value: 255
Logical Address: 60230, Physical Address: 60230, Value: 255
Logical Address: 55595, Physical Address: 55595, Value: 255
Logical Address: 35295, Physical Address: 35295, Value: 255
Logical Address: 36260, Physical Address: 36260, Value: 255
Logical Address: 19895, Physical Address: 19895, Value: 255
Logical Address: 47195, Physical Address: 47195, Value: 255
Logical Address: 38095, Physical Address: 38095, Value: 255
Logical Address: 29525, Physical Address: 29525, Value: 255
Logical Address: 295, Physical Address: 295, Value: 255
Logical Address: 59530, Physical Address: 59530, Value: 255
Logical Address: 61630, Physical Address: 61630, Value: 255
Logical Address: 60930, Physical Address: 60930, Value: 255
Logical Address: 49560, Physical Address: 49560, Value: 255
Logical Address: 9660, Physical Address: 9660, Value: 255
Logical Address: 22960, Physical Address: 22960, Value: 255
Logical Address: 12725, Physical Address: 12725, Value: 255
Logical Address: 5895, Physical Address: 5895, Value: 255
Logical Address: 46060, Physical Address: 46060, Value: 255
Logical Address: 5025, Physical Address: 5025, Value: 255
Logical Address: 2660, Physical Address: 2660, Value: 255
Logical Address: 31095, Physical Address: 31095, Value: 255
Logical Address: 37660, Physical Address: 37660, Value: 255
Logical Address: 7825, Physical Address: 7825, Value: 255
Logical Address: 61461, Physical Address: 61461, Value: 255
Logical Address: 11495, Physical Address: 11495, Value: 255
Logical Address: 49995, Physical Address: 49995, Value: 255
Logical Address: 31360, Physical Address: 31360, Value: 255
Logical Address: 9925, Physical Address: 9925, Value: 255
Logical Address: 60761, Physical Address: 60761, Value: 255
Logical Address: 125, Physical Address: 125, Value: 255
Logical Address: 10095, Physical Address: 10095, Value: 255
Logical Address: 44660, Physical Address: 44660, Value: 255
Logical Address: 17095, Physical Address: 17095, Value: 255
Logical Address: 53930, Physical Address: 53930, Value: 255
Logical Address: 57695, Physical Address: 57695, Value: 255
Logical Address: 34595, Physical Address: 34595, Value: 255
Logical Address: 42295, Physical Address: 42295, Value: 255
Logical Address: 28125, Physical Address: 28125, Value: 255
Logical Address: 60496, Physical Address: 60496, Value: 255
Logical Address: 27860, Physical Address: 27860, Value: 255
Logical Address: 22695, Physical Address: 22695, Value: 255
Logical Address: 40460, Physical Address: 40460, Value: 255
Logical Address: 18495, Physical Address: 18495, Value: 255
Logical Address: 41860, Physical Address: 41860, Value: 255
Logical Address: 6425, Physical Address: 6425, Value: 255
Logical Address: 7995, Physical Address: 7995, Value: 255
Logical Address: 45360, Physical Address: 45360, Value: 255
Logical Address: 11060, Physical Address: 11060, Value: 255
Logical Address: 28295, Physical Address: 28295, Value: 255
Logical Address: 44395, Physical Address: 44395, Value: 255
Logical Address: 27425, Physical Address: 27425, Value: 255
Logical Address: 995, Physical Address: 995, Value: 255
Logical Address: 53495, Physical Address: 53495, Value: 255
Logical Address: 35995, Physical Address: 35995, Value: 255
Logical Address: 30225, Physical Address: 30225, Value: 255
Logical Address: 59796, Physical Address: 59796, Value: 255
Logical Address: 33895, Physical Address: 33895, Value: 255
Logical Address: 30925, Physical Address: 30925, Value: 255
Logical Address: 53761, Physical Address: 53761, Value: 255
Logical Address: 8260, Physical Address: 8260, Value: 255
Logical Address: 17625, Physical Address: 17625, Value: 255
Logical Address: 63996, Physical Address: 63996, Value: 255
Logical Address: 32495, Physical Address: 32495, Value: 255
Logical Address: 42995, Physical Address: 42995, Value: 255
Logical Address: 39495, Physical Address: 39495, Value: 255
Logical Address: 34860, Physical Address: 34860, Value: 255
Logical Address: 14560, Physical Address: 14560, Value: 255
Logical Address: 15525, Physical Address: 15525, Value: 255
Logical Address: 64696, Physical Address: 64696, Value: 255
Logical Address: 26460, Physical Address: 26460, Value: 255
Logical Address: 17360, Physical Address: 17360, Value: 255
Logical Address: 26195, Physical Address: 26195, Value: 255
Logical Address: 45095, Physical Address: 45095, Value: 255
Logical Address: 38795, Physical Address: 38795, Value: 255
Logical Address: 40895, Physical Address: 40895, Value: 255
Logical Address: 40195, Physical Address: 40195, Value: 255
Logical Address: 28825, Physical Address: 28825, Value: 255
Logical Address: 54461, Physical Address: 54461, Value: 255
Logical Address: 2225, Physical Address: 2225, Value: 255
Logical Address: 9395, Physical Address: 9395, Value: 255
Logical Address: 50695, Physical Address: 50695, Value: 255
Logical Address: 25325, Physical Address: 25325, Value: 255
Logical Address: 1695, Physical Address: 1695, Value: 255
Logical Address: 47460, Physical Address: 47460, Value: 255
Logical Address: 10360, Physical Address: 10360, Value: 255
Logical Address: 16925, Physical Address: 16925, Value: 255
Logical Address: 4495, Physical Address: 4495, Value: 255
Logical Address: 58130, Physical Address: 58130, Value: 255
Logical Address: 56295, Physical Address: 56295, Value: 255
Logical Address: 29260, Physical Address: 29260, Value: 255
Logical Address: 10625, Physical Address: 10625, Value: 255
Logical Address: 6595, Physical Address: 6595, Value: 255
Logical Address: 57430, Physical Address: 57430, Value: 255
Logical Address: 62330, Physical Address: 62330, Value: 255
Logical Address: 54895, Physical Address: 54895, Value: 255
Logical Address: 23925, Physical Address: 23925, Value: 255
Logical Address: 61896, Physical Address: 61896, Value: 255
Logical Address: 33195, Physical Address: 33195, Value: 255
Logical Address: 36960, Physical Address: 36960, Value: 255
Logical Address: 13860, Physical Address: 13860, Value: 255
Logical Address: 21560, Physical Address: 21560, Value: 255
Logical Address: 24795, Physical Address: 24795, Value: 255
Logical Address: 39760, Physical Address: 39760, Value: 255
Logical Address: 7125, Physical Address: 7125, Value: 255
Logical Address: 1960, Physical Address: 1960, Value: 255
Logical Address: 19725, Physical Address: 19725, Value: 255
Logical Address: 63296, Physical Address: 63296, Value: 255
Logical Address: 21125, Physical Address: 21125, Value: 255
Logical Address: 3095, Physical Address: 3095, Value: 255
Logical Address: 52795, Physical Address: 52795, Value: 255
Logical Address: 24625, Physical Address: 24625, Value: 255
Logical Address: 55861, Physical Address: 55861, Value: 255
Logical Address: 7560, Physical Address: 7560, Value: 255
Logical Address: 23660, Physical Address: 23660, Value: 255
Logical Address: 24095, Physical Address: 24095, Value: 255
Logical Address: 45795, Physical Address: 45795, Value: 255
Logical Address: 32760, Physical Address: 32760, Value: 255
Logical Address: 15260, Physical Address: 15260, Value: 255
Logical Address: 26895, Physical Address: 26895, Value: 255
Logical Address: 39060, Physical Address: 39060, Value: 255
Logical Address: 13160, Physical Address: 13160, Value: 255
Logical Address: 27595, Physical Address: 27595, Value: 255
Logical Address: 33025, Physical Address: 33025, Value: 255
Logical Address: 53061, Physical Address: 53061, Value: 255
Logical Address: 14295, Physical Address: 14295, Value: 255
Logical Address: 43260, Physical Address: 43260, Value: 255
Logical Address: 11760, Physical Address: 11760, Value: 255
Logical Address: 22260, Physical Address: 22260, Value: 255
Logical Address: 18760, Physical Address: 18760, Value: 255
Logical Address: 14125, Physical Address: 14125, Value: 255
Logical Address: 59361, Physical Address: 59361, Value: 255
Logical Address: 12195, Physical Address: 12195, Value: 255
Logical Address: 43960, Physical Address: 43960, Value: 255
Logical Address: 5725, Physical Address: 5725, Value: 255
Logical Address: 62161, Physical Address: 62161, Value: 255
Logical Address: 5460, Physical Address: 5460, Value: 255
Logical Address: 24360, Physical Address: 24360, Value: 255
Logical Address: 18060, Physical Address: 18060, Value: 255
Logical Address: 20160, Physical Address: 20160, Value: 255
Logical Address: 19460, Physical Address: 19460, Value: 255
Logical Address: 25495, Physical Address: 25495, Value: 255
Logical Address: 51130, Physical Address: 51130, Value: 255
Logical Address: 64430, Physical Address: 64430, Value: 255
Logical Address: 54195, Physical Address: 54195, Value: 255
Logical Address: 29960, Physical Address: 29960, Value: 255
Logical Address: 21995, Physical Address: 21995, Value: 255
Logical Address: 46495, Physical Address: 46495, Value: 255
Logical Address: 26725, Physical Address: 26725, Value: 255
Logical Address: 55161, Physical Address: 55161, Value: 255
Logical Address: 13595, Physical Address: 13595, Value: 255
Logical Address: 49295, Physical Address: 49295, Value: 255
Logical Address: 37395, Physical Address: 37395, Value: 255
Logical Address: 35560, Physical Address: 35560, Value: 255
Logical Address: 8525, Physical Address: 8525, Value: 255
Logical Address: 7295, Physical Address: 7295, Value: 255
Logical Address: 51395, Physical Address: 51395, Value: 255
Logical Address: 36695, Physical Address: 36695, Value: 255
Logical Address: 41595, Physical Address: 41595, Value: 255
Logical Address: 34160, Physical Address: 34160, Value: 255
Logical Address: 20595, Physical Address: 20595, Value: 255
Logical Address: 41160, Physical Address: 41160, Value: 255
Logical Address: 12460, Physical Address: 12460, Value: 255
Logical Address: 16225, Physical Address: 16225, Value: 255
Logical Address: 58661, Physical Address: 58661, Value: 255
Logical Address: 825, Physical Address: 825, Value: 255
Logical Address: 4060, Physical Address: 4060, Value: 255
Logical Address: 19025, Physical Address: 19025, Value: 255
Logical Address: 3795, Physical Address: 3795, Value: 255
Logical Address: 46760, Physical Address: 46760, Value: 255
Logical Address: 16395, Physical Address: 16395, Value: 255
Logical Address: 42560, Physical Address: 42560, Value: 255
Logical Address: 17795, Physical Address: 17795, Value: 255
Logical Address: 47895, Physical Address: 47895, Value: 255
Logical Address: 32060, Physical Address: 32060, Value: 255
Logical Address: 21295, Physical Address: 21295, Value: 255
Logical Address: 52530, Physical Address: 52530, Value: 255
Logical Address: 52360, Physical Address: 52360, Value: 255
Logical Address: 2925, Physical Address: 2925, Value: 255
Logical Address: 3360, Physical Address: 3360, Value: 255
Logical Address: 25060, Physical Address: 25060, Value: 255
Logical Address: 12025, Physical Address: 12025, Value: 255
Logical Address: 60061, Physical Address: 60061, Value: 255
Logical Address: 6160, Physical Address: 6160, Value: 255
Logical Address: 18325, Physical Address: 18325, Value: 255
Logical Address: 57961, Physical Address: 57961, Value: 255
Logical Address: 6860, Physical Address: 6860, Value: 255
Logical Address: 29695, Physical Address: 29695, Value: 255
Logical Address: 32325, Physical Address: 32325, Value: 255
Logical Address: 59096, Physical Address: 59096, Value: 255
Logical Address: 22525, Physical Address: 22525, Value: 255
Logical Address: 56561, Physical Address: 56561, Value: 255
Logical Address: 1525, Physical Address: 1525, Value: 255
Logical Address: 63561, Physical Address: 63561, Value: 255
Logical Address: 10795, Physical Address: 10795, Value: 255
Logical Address: 56030, Physical Address: 56030, Value: 255
Logical Address: 56995, Physical Address: 56995, Value: 255
Logical Address: 23225, Physical Address: 23225, Value: 255
Logical Address: 2395, Physical Address: 2395, Value: 255
Logical Address: 58830, Physical Address: 58830, Value: 255
Logical Address: 50260, Physical Address: 50260, Value: 255
Logical Address: 3625, Physical Address: 3625, Value: 255
Logical Address: 62861, Physical Address: 62861, Value: 255
Logical Address: 64961, Physical Address: 64961, Value: 255
Logical Address: 64261, Physical Address: 64261, Value: 255
Logical Address: 4760, Physical Address: 4760, Value: 255
Logical Address: 30395, Physical Address: 30395, Value: 255
Logical Address: 43695, Physical Address: 43695, Value: 255
Logical Address: 33460, Physical Address: 33460, Value: 255
Logical Address: 9225, Physical Address: 9225, Value: 255
Logical Address: 1260, Physical Address: 1260, Value: 255
Logical Address: 25760, Physical Address: 25760, Value: 255
Logical Address: 23395, Physical Address: 23395, Value: 255
Logical Address: 51830, Physical Address: 51830, Value: 255
Logical Address: 58395, Physical Address: 58395, Value: 255
Logical Address: 28560, Physical Address: 28560, Value: 255
Logical Address: 16660, Physical Address: 16660, Value: 255
Logical Address: 14825, Physical Address: 14825, Value: 255
Logical Address: 5195, Physical Address: 5195, Value: 255
Logical Address: 52095, Physical Address: 52095, Value: 255
Logical Address: 30660, Physical Address: 30660, Value: 255
Logical Address: 15960, Physical Address: 15960, Value: 255
Logical Address: 20860, Physical Address: 20860, Value: 255
Logical Address: 13425, Physical Address: 13425, Value: 255
Logical Address: 65396, Physical Address: 65396, Value: 255
Logical Address: 20425, Physical Address: 20425, Value: 255
Logical Address: 57261, Physical Address: 57261, Value: 255
Logical Address: 12895, Physical Address: 12895, Value: 255
Logical Address: 55330, Physical Address: 55330, Value: 255
Logical Address: 63030, Physical Address: 63030, Value: 255
Logical Address: 48860, Physical Address: 48860, Value: 255
Logical Address: 15695, Physical Address: 15695, Value: 255
Logical Address: 48595, Physical Address: 48595, Value: 255
Logical Address: 26025, Physical Address: 26025, Value: 255
Logical Address: 61196, Physical Address: 61196, Value: 255
Logical Address: 21825, Physical Address: 21825, Value: 255
Logical Address: 62596, Physical Address: 62596, Value: 255
Logical Address: 27160, Physical Address: 27160, Value: 255
Logical Address: 11325, Physical Address: 11325, Value: 255
Logical Address: 560, Physical Address: 560, Value: 255
Logical Address: 31795, Physical Address: 31795, Value: 255
Logical Address: 31625, Physical Address: 31625, Value: 255
Logical Address: 65131, Physical Address: 65131, Value: 255
Logical Address: 48160, Physical Address: 48160, Value: 255
Logical Address: 4325, Physical Address: 4325, Value: 255
Logical Address: 8695, Physical Address: 8695, Value: 255
Logical Address: 56730, Physical Address: 56730, Value: 255
Logical Address: 50960, Physical Address: 50960, Value: 255
Logical Address: 14995, Physical Address: 14995, Value: 255
Logical Address: 54630, Physical Address: 54630, Value: 255
Logical Address: 51660, Physical Address: 51660, Value: 255
Logical Address: 8960, Physical Address: 8960, Value: 255
Logical Address: 28995, Physical Address: 28995, Value: 255
Logical Address: 38360, Physical Address: 38360, Value: 255
Logical Address: 19195, Physical Address: 19195, Value: 255
Logical Address: 53230, Physical Address: 53230, Value: 255
Logical Address: 63730, Physical Address: 63730, Value: 255
Logical Address: 60230, Physical Address: 60230, Value: 255
Logical Address: 55595, Physical Address: 55595, Value: 255
Logical Address: 35295, Physical Address: 35295, Value: 255
Logical Address: 36260, Physical Address: 36260, Value: 255
Logical Address: 19895, Physical Address: 19895, Value: 255
Logical Address: 47195, Physical Address: 47195, Value: 255
Logical Address: 38095, Physical Address: 38095, Value: 255
Logical Address: 29525, Physical Address: 29525, Value: 255
Logical Address: 295, Physical Address: 295, Value: 255
Logical Address: 59530, Physical Address: 59530, Value: 255
Logical Address: 61630, Physical Address: 61630, Value: 255
Logical Address: 60930, Physical Address: 60930, Value: 255
Logical Address: 49560, Physical Address: 49560, Value: 255
Logical Address: 9660, Physical Address: 9660, Value: 255
Logical Address: 22960, Physical Address: 22960, Value: 255
Logical Address: 12725, Physical Address: 12725, Value: 255
Logical Address: 5895, Physical Address: 5895, Value: 255
Logical Address: 46060, Physical Address: 46060, Value: 255
Logical Address: 5025, Physical Address: 5025, Value: 255
Logical Address: 2660, Physical Address: 2660, Value: 255
Logical Address: 31095, Physical Address: 31095, Value: 255
Logical Address: 37660, Physical Address: 37660, Value: 255
Logical Address: 7825, Physical Address: 7825, Value: 255
Logical Address: 61461, Physical Address: 61461, Value: 255
Logical Address: 11495, Physical Address: 11495, Value: 255
Logical Address: 49995, Physical Address: 49995, Value: 255
Logical Address: 31360, Physical Address: 31360, Value: 255
Logical Address: 9925, Physical Address: 9925, Value: 255
Logical Address: 60761, Physical Address: 60761, Value: 255
Logical Address: 125, Physical Address: 125, Value: 255
Logical Address: 10095, Physical Address: 10095, Value: 255
Logical Address: 44660, Physical Address: 44660, Value: 255
Logical Address: 17095, Physical Address: 17095, Value: 255
Logical Address: 53930, Physical Address: 53930, Value: 255
Logical Address: 57695, Physical Address: 57695, Value: 255
Logical Address: 34595, Physical Address: 34595, Value: 255
Logical Address: 42295, Physical Address: 42295, Value: 255
Logical Address: 28125, Physical Address: 28125, Value: 255
Logical Address: 60496, Physical Address: 60496, Value: 255
Logical Address: 27860, Physical Address: 27860, Value: 255
Logical Address: 22695, Physical Address: 22695, Value: 255
Logical Address: 40460, Physical Address: 40460, Value: 255
Logical Address: 18495, Physical Address: 18495, Value: 255
Logical Address: 41860, Physical Address: 41860, Value: 255
Logical Address: 6425, Physical Address: 6425, Value: 255
Logical Address: 7995, Physical Address: 7995, Value: 255
Logical Address: 45360, Physical Address: 45360, Value: 255
Logical Address: 11060, Physical Address: 11060, Value: 255
Logical Address: 28295, Physical Address: 28295, Value: 255
Logical Address: 44395, Physical Address: 44395, Value: 255
Logical Address: 27425, Physical Address: 27425, Value: 255
Logical Address: 995, Physical Address: 995, Value: 255
Logical Address: 53495, Physical Address: 53495, Value: 255
Logical Address: 35995, Physical Address: 35995, Value: 255
Logical Address: 30225, Physical Address: 30225, Value: 255
Logical Address: 59796, Physical Address: 59796, Value: 255
Logical Address: 33895, Physical Address: 33895, Value: 255
Logical Address: 30925, Physical Address: 30925, Value: 255
Logical Address: 53761, Physical Address: 53761, Value: 255
Logical Address: 8260, Physical Address: 8260, Value: 255
Logical Address: 17625, Physical Address: 17625, Value: 255
Logical Address: 63996, Physical Address: 63996, Value: 255
Logical Address: 32495, Physical Address: 32495, Value: 255
Logical Address: 42995, Physical Address: 42995, Value: 255
Logical Address: 39495, Physical Address: 39495, Value: 255
Logical Address: 34860, Physical Address: 34860, Value: 255
Logical Address: 14560, Physical Address: 14560, Value: 255
Logical Address: 15525, Physical Address: 15525, Value: 255
Logical Address: 64696, Physical Address: 64696, Value: 255
Logical Address: 26460, Physical Address: 26460, Value: 255
Logical Address: 17360, Physical Address: 17360, Value: 255
Logical Address: 26195, Physical Address: 26195, Value: 255
Logical Address: 45095, Physical Address: 45095, Value: 255
Logical Address: 38795, Physical Address: 38795, Value: 255
Logical Address: 40895, Physical Address: 40895, Value: 255
Logical Address: 40195, Physical Address: 40195, Value: 255
Logical Address: 28825, Physical Address: 28825, Value: 255
Logical Address: 54461, Physical Address: 54461, Value: 255
Logical Address: 2225, Physical Address: 2225, Value: 255
Logical Address: 9395, Physical Address: 9395, Value: 255
Logical Address: 50695, Physical Address: 50695, Value: 255
Logical Address: 25325, Physical Address: 25325, Value: 255
Logical Address: 1695, Physical Address: 1695, Value: 255
Logical Address: 47460, Physical Address: 47460, Value: 255
Logical Address: 10360, Physical Address: 10360, Value: 255
Logical Address: 16925, Physical Address: 16925, Value: 255
Logical Address: 4495, Physical Address: 4495, Value: 255
Logical Address: 58130, Physical Address: 58130, Value: 255
Logical Address: 56295, Physical Address: 56295, Value: 255
Logical Address: 29260, Physical Address: 29260, Value: 255
Logical Address: 10625, Physical Address: 10625, Value: 255
Logical Address: 6595, Physical Address: 6595, Value: 255
Logical Address: 57430, Physical Address: 57430, Value: 255
Logical Address: 62330, Physical Address: 62330, Value: 255
Logical Address: 54895, Physical Address: 54895, Value: 255
Logical Address: 23925, Physical Address: 23925, Value: 255
Logical Address: 61896, Physical Address: 61896, Value: 255
Logical Address: 33195, Physical Address: 33195, Value: 255
Logical Address: 36960, Physical Address: 36960, Value: 255
Logical Address: 13860, Physical Address: 13860, Value: 255
Logical Address: 21560, Physical Address: 21560, Value: 255
Logical Address: 24795, Physical Address: 24795, Value: 255
Logical Address: 39760, Physical Address: 39760, Value: 255
Logical Address: 7125, Physical Address: 7125, Value: 255
Logical Address: 1960, Physical Address: 1960, Value: 255
Logical Address: 19725, Physical Address: 19725, Value: 255
Logical Address: 63296, Physical Address: 63296, Value: 255
Logical Address: 21125, Physical Address: 21125, Value: 255
Logical Address: 3095, Physical Address: 3095, Value: 255
Logical Address: 52795, Physical Address: 52795, Value: 255
Logical Address: 24625, Physical Address: 24625, Value: 255
Logical Address: 55861, Physical Address: 55861, Value: 255
Logical Address: 7560, Physical Address: 7560, Value: 255
Logical Address: 23660, Physical Address: 23660, Value: 255
Logical Address: 24095, Physical Address: 24095, Value: 255
Logical Address: 45795, Physical Address: 45795, Value: 255
Logical Address: 32760, Physical Address: 32760, Value: 255
Logical Address: 15260, Physical Address: 15260, Value: 255
Logical Address: 26895, Physical Address: 26895, Value: 255
Logical Address: 39060, Physical Address: 39060, Value: 255
Logical Address: 13160, Physical Address: 13160, Value: 255
Logical Address: 27595, Physical Address: 27595, Value: 255
Logical Address: 33025, Physical Address: 33025, Value: 255
Logical Address: 53061, Physical Address: 53061, Value: 255
Logical Address: 14295, Physical Address: 14295, Value: 255
Logical Address: 43260, Physical Address: 43260, Value: 255
Logical Address: 11760, Physical Address: 11760, Value: 255
Logical Address: 22260, Physical Address: 22260, Value: 255
Logical Address: 18760, Physical Address: 18760, Value: 255
Logical Address: 14125, Physical Address: 14125, Value: 255
Logical Address: 59361, Physical Address: 59361, Value: 255
Logical Address: 12195, Physical Address: 12195, Value: 255
Logical Address: 43960, Physical Address: 43960, Value: 255
Logical Address: 5725, Physical Address: 5725, Value: 255
Logical Address: 62161, Physical Address: 62161, Value: 255
Logical Address: 5460, Physical Address: 5460, Value: 255
Logical Address: 24360, Physical Address: 24360, Value: 255
Logical Address: 18060, Physical Address: 18060, Value: 255
Logical Address: 20160, Physical Address: 20160, Value: 255
Logical Address: 19460, Physical Address: 19460, Value: 255
Logical Address: 25495, Physical Address: 25495, Value: 255
Logical Address: 51130, Physical Address: 51130, Value: 255
Logical Address: 64430, Physical Address: 64430, Value: 255
Logical Address: 54195, Physical Address: 54195, Value: 255
Logical Address: 29960, Physical Address: 29960, Value: 255
Logical Address: 21995, Physical Address: 21995, Value: 255
Logical Address: 46495, Physical Address: 46495, Value: 255
Logical Address: 26725, Physical Address: 26725, Value: 255
Logical Address: 55161, Physical Address: 55161, Value: 255
Logical Address: 13595, Physical Address: 13595, Value: 255
Logical Address: 49295, Physical Address: 49295, Value: 255
Logical Address: 37395, Physical Address: 37395, Value: 255
Logical Address: 35560, Physical Address: 35560, Value: 255
Logical Address: 8525, Physical Address: 8525, Value: 255
Logical Address: 7295, Physical Address: 7295, Value: 255
Logical Address: 51395, Physical Address: 51395, Value: 255
Logical Address: 36695, Physical Address: 36695, Value: 255
Logical Address: 41595, Physical Address: 41595, Value: 255
Logical Address: 34160, Physical Address: 34160, Value: 255
Logical Address: 20595, Physical Address: 20595, Value: 255
Logical Address: 41160, Physical Address: 41160, Value: 255
Logical Address: 12460, Physical Address: 12460, Value: 255
Logical Address: 16225, Physical Address: 16225, Value: 255
Logical Address: 4060, Physical Address: 4060, Value: 255
Logical Address: 19025, Physical Address: 19025, Value: 255
Logical Address: 3795, Physical Address: 3795, Value: 255
Logical Address: 46760, Physical Address: 46760, Value: 255
Logical Address: 16395, Physical Address: 16395, Value: 255
Logical Address: 42560, Physical Address: 42560, Value: 255
Logical Address: 17795, Physical Address: 17795, Value: 255
Logical Address: 47895, Physical Address: 47895, Value: 255
Logical Address: 32060, Physical Address: 32060, Value: 255
Logical Address: 21295, Physical Address: 21295, Value: 255
Logical Address: 52530, Physical Address: 52530, Value: 255
Logical Address: 52360, Physical Address: 52360, Value: 255
Logical Address: 2925, Physical Address: 2925, Value: 255
Logical Address: 3360, Physical Address: 3360, Value: 255
Logical Address: 25060, Physical Address: 25060, Value: 255
Logical Address: 12025, Physical Address: 12025, Value: 255
Logical Address: 60061, Physical Address: 60061, Value: 255
Logical Address: 6160, Physical Address: 6160, Value: 255
Logical Address: 18325, Physical Address: 18325, Value: 255
Logical Address: 57961, Physical Address: 57961, Value: 255
Logical Address: 6860, Physical Address: 6860, Value: 255
Logical Address: 29695, Physical Address: 29695, Value: 255
Logical Address: 32325, Physical Address: 32325, Value: 255
Logical Address: 59096, Physical Address: 59096, Value: 255
Logical Address: 22525, Physical Address: 22525, Value: 255
Logical Address: 56561, Physical Address: 56561, Value: 255
Logical Address: 1525, Physical Address: 1525, Value: 255
Logical Address: 63561, Physical Address: 63561, Value: 255
Logical Address: 10795, Physical Address: 10795, Value: 255
Logical Address: 56030, Physical Address: 56030, Value: 255
Logical Address: 56995, Physical Address: 56995, Value: 255
Logical Address: 23225, Physical Address: 23225, Value: 255
Logical Address: 2395, Physical Address: 2395, Value: 255
Logical Address: 58830, Physical Address: 58830, Value: 255
Logical Address: 50260, Physical Address: 50260, Value: 255
Logical Address: 3625, Physical Address: 3625, Value: 255
Logical Address: 62861, Physical Address: 62861, Value: 255
Logical Address: 64961, Physical Address: 64961, Value: 255
Logical Address: 64261, Physical Address: 64261, Value: 255
Logical Address: 4760, Physical Address: 4760, Value: 255
Logical Address: 30395, Physical Address: 30395, Value: 255
Logical Address: 43695, Physical Address: 43695, Value: 255
Logical Address: 33460, Physical Address: 33460, Value: 255
Logical Address: 9225, Physical Address: 9225, Value: 255
Logical Address: 1260, Physical Address: 1260, Value: 255
Logical Address: 25760, Physical Address: 25760, Value: 255
Logical Address: 23395, Physical Address: 23395, Value: 255
Logical Address: 51830, Physical Address: 51830, Value: 255
Logical Address: 58395, Physical Address: 58395, Value: 255
Logical Address: 28560, Physical Address: 28560, Value: 255
Logical Address: 16660, Physical Address: 16660, Value: 255
Logical Address: 14825, Physical Address: 14825, Value: 255
Logical Address: 5195, Physical Address: 5195, Value: 255
Logical Address: 52095, Physical Address: 52095, Value: 255
Logical Address: 30660, Physical Address: 30660, Value: 255
Logical Address: 15960, Physical Address: 15960, Value: 255
Logical Address: 20860, Physical Address: 20860, Value: 255
Logical Address: 13425, Physical Address: 13425, Value: 255
Logical Address: 65396, Physical Address: 65396, Value: 255
Logical Address: 20425, Physical Address: 20425, Value: 255
Logical Address: 57261, Physical Address: 57261, Value: 255
Logical Address: 12895, Physical Address: 12895, Value: 255
Logical Address: 55330, Physical Address: 55330, Value: 255
Logical Address: 63030, Physical Address: 63030, Value: 255
Logical Address: 48860, Physical Address: 48860, Value: 255
Logical Address: 15695, Physical Address: 15695, Value: 255
Logical Address: 48595, Physical Address: 48595, Value: 255
Logical Address: 26025, Physical Address: 26025, Value: 255
Logical Address: 61196, Physical Address: 61196, Value: 255
Logical Address: 21825, Physical Address: 21825, Value: 255
Logical Address: 62596, Physical Address: 62596, Value: 255
Logical Address: 27160, Physical Address: 27160, Value: 255
Logical Address: 11325, Physical Address: 11325, Value: 255
Logical Address: 560, Physical Address: 560, Value: 255
Logical Address: 31795, Physical Address: 31795, Value: 255
Logical Address: 31625, Physical Address: 31625, Value: 255
Logical Address: 65131, Physical Address: 65131, Value: 255
Logical Address: 48160, Physical Address: 48160, Value: 255
Logical Address: 4325, Physical Address: 4325, Value: 255
Logical Address: 8695, Physical Address: 8695, Value: 255
Logical Address: 56730, Physical Address: 56730, Value: 255
Logical Address: 50960, Physical Address: 50960, Value: 255
Logical Address: 14995, Physical Address: 14995, Value: 255
Logical Address: 54630, Physical Address: 54630, Value: 255
Logical Address: 51660, Physical Address: 51660, Value: 255
Logical Address: 8960, Physical Address: 8960, Value: 255
Logical Address: 28995, Physical Address: 28995, Value: 255
Logical Address: 38360, Physical Address: 38360, Value: 255
Logical Address: 19195, Physical Address: 19195, Value: 255
Logical Address: 53230, Physical Address: 53230, Value: 255
Logical Address: 63730, Physical Address: 63730, Value: 255
Logical Address: 60230, Physical Address: 60230, Value: 255
Logical Address: 55595, Physical Address: 55595, Value: 255
Logical Address: 35295, Physical Address: 35295, Value: 255
Logical Address: 36260, Physical Address: 36260, Value: 255
Logical Address: 19895, Physical Address: 19895, Value: 255
Logical Address: 47195, Physical Address: 47195, Value: 255
Logical Address: 38095, Physical Address: 38095, Value: 255
Logical Address: 29525, Physical Address: 29525, Value: 255
Logical Address: 295, Physical Address: 295, Value: 255
Logical Address: 59530, Physical Address: 59530, Value: 255
Logical Address: 61630, Physical Address: 61630, Value: 255
Logical Address: 60930, Physical Address: 60930, Value: 255
Logical Address: 49560, Physical Address: 49560, Value: 255
Logical Address: 9660, Physical Address: 9660, Value: 255
Logical Address: 22960, Physical Address: 22960, Value: 255
Logical Address: 12725, Physical Address: 12725, Value: 255
Logical Address: 5895, Physical Address: 5895, Value: 255
Logical Address: 46060, Physical Address: 46060, Value: 255
Logical Address: 5025, Physical Address: 5025, Value: 255
Logical Address: 2660, Physical Address: 2660, Value: 255
Logical Address: 31095, Physical Address: 31095, Value: 255
Logical Address: 37660, Physical Address: 37660, Value: 255
Logical Address: 7825, Physical Address: 7825, Value: 255
Logical Address: 61461, Physical Address: 61461, Value: 255
Logical Address: 11495, Physical Address: 11495, Value: 255
Logical Address: 49995, Physical Address: 49995, Value: 255
Logical Address: 31360, Physical Address: 31360, Value: 255
Logical Address: 9925, Physical Address: 9925, Value: 255
Logical Address: 60761, Physical Address: 60761, Value: 255
Logical Address: 125, Physical Address: 125, Value: 255
Logical Address: 10095, Physical Address: 10095, Value: 255
Logical Address: 44660, Physical Address: 44660, Value: 255
Logical Address: 17095, Physical Address: 17095, Value: 255
Logical Address: 53930, Physical Address: 53930, Value: 255
Logical Address: 57695, Physical Address: 57695, Value: 255
Logical Address: 34595, Physical Address: 34595, Value: 255
Logical Address: 42295, Physical Address: 42295, Value: 255
Logical Address: 28125, Physical Address: 28125, Value: 255
Logical Address: 60496, Physical Address: 60496, Value: 255
Logical Address: 27860, Physical Address: 27860, Value: 255
Logical Address: 22695, Physical Address: 22695, Value: 255
Logical Address: 40460, Physical Address: 40460, Value: 255
Logical Address: 18495, Physical Address: 18495, Value: 255
Logical Address: 41860, Physical Address: 41860, Value: 255
Logical Address: 6425, Physical Address: 6425, Value: 255
Logical Address: 7995, Physical Address: 7995, Value: 255
Logical Address: 45360, Physical Address: 45360, Value: 255
Logical Address: 11060, Physical Address: 11060, Value: 255
Logical Address: 28295, Physical Address: 28295, Value: 255
Logical Address: 44395, Physical Address: 44395, Value: 255
Logical Address: 27425, Physical Address: 27425, Value: 255
Logical Address: 995, Physical Address: 995, Value: 255
Logical Address: 53495, Physical Address: 53495, Value: 255
Logical Address: 35995, Physical Address: 35995, Value: 255
Logical Address: 30225, Physical Address: 30225, Value: 255
Logical Address: 59796, Physical Address: 59796, Value: 255
Logical Address: 33895, Physical Address: 33895, Value: 255
Logical Address: 30925, Physical Address: 30925, Value: 255
Logical Address: 53761, Physical Address: 53761, Value: 255
Logical Address: 8260, Physical Address: 8260, Value: 255
Logical Address: 17625, Physical Address: 17625, Value: 255
Logical Address: 63996, Physical Address: 63996, Value: 255
Logical Address: 32495, Physical Address: 32495, Value: 255
Logical Address: 42995, Physical Address: 42995, Value: 255
Logical Address: 39495, Physical Address: 39495, Value: 255
Logical Address: 34860, Physical Address: 34860, Value: 255
Logical Address: 14560, Physical Address: 14560, Value: 255
Logical Address: 15525, Physical Address: 15525, Value: 255
Logical Address: 64696, Physical Address: 64696, Value: 255
Logical Address: 26460, Physical Address: 26460, Value: 255
Logical Address: 17360, Physical Address: 17360, Value: 255
Logical Address: 26195, Physical Address: 26195, Value: 255
Logical Address: 45095, Physical Address: 45095, Value: 255
Logical Address: 38795, Physical Address: 38795, Value: 255
Logical Address: 40895, Physical Address: 40895, Value: 255
Logical Address: 40195, Physical Address: 40195, Value: 255
Logical Address: 28825, Physical Address: 28825, Value: 255
Logical Address: 54461, Physical Address: 54461, Value: 255
Logical Address: 2225, Physical Address: 2225, Value: 255
Logical Address: 9395, Physical Address: 9395, Value: 255
Logical Address: 50695, Physical Address: 50695, Value: 255
Logical Address: 25325, Physical Address: 25325, Value: 255
Logical Address: 1695, Physical Address: 1695, Value: 255
Logical Address: 47460, Physical Address: 47460, Value: 255
Logical Address: 10360, Physical Address: 10360, Value: 255
Logical Address: 16925, Physical Address: 16925, Value: 255
Logical Address: 4495, Physical Address: 4495, Value: 255
Logical Address: 58130, Physical Address: 58130, Value: 255
Logical Address: 56295, Physical Address: 56295, Value: 255
Logical Address: 29260, Physical Address: 29260, Value: 255
Logical Address: 10625, Physical Address: 10625, Value: 255
Logical Address: 6595, Physical Address: 6595, Value: 255
Logical Address: 57430, Physical Address: 57430, Value: 255
Logical Address: 62330, Physical Address: 62330, Value: 255
Logical Address: 54895, Physical Address: 54895, Value: 255
Logical Address: 23925, Physical Address: 23925, Value: 255
Logical Address: 61896, Physical Address: 61896, Value: 255
Logical Address: 33195, Physical Address: 33195, Value: 255
Logical Address: 36960, Physical Address: 36960, Value: 255
Logical Address: 13860, Physical Address: 13860, Value: 255
Logical Address: 21560, Physical Address: 21560, Value: 255
Logical Address: 24795, Physical Address: 24795, Value: 255
Logical Address: 39760, Physical Address: 39760, Value: 255
Logical Address: 7125, Physical Address: 7125, Value: 255
Logical Address: 1960, Physical Address: 1960, Value: 255
Logical Address: 19725, Physical Address: 19725, Value: 255
Logical Address: 63296, Physical Address: 63296, Value: 255
Logical Address: 21125, Physical Address: 21125, Value: 255
Logical Address: 3095, Physical Address: 3095, Value: 255
Logical Address: 52795, Physical Address: 52795, Value: 255
Logical Address: 24625, Physical Address: 24625, Value: 255
Logical Address: 55861, Physical Address: 55861, Value: 255
Logical Address: 7560, Physical Address: 7560, Value: 255
Logical Address: 23660, Physical Address: 23660, Value: 255
Logical Address: 24095, Physical Address: 24095, Value: 255
Logical Address: 45795, Physical Address: 45795, Value: 255
Logical Address: 32760, Physical Address: 32760, Value: 255
Logical Address: 15260, Physical Address: 15260, Value: 255
Logical Address: 26895, Physical Address: 26895, Value: 255
Logical Address: 39060, Physical Address: 39060, Value: 255
Logical Address: 13160, Physical Address: 13160, Value: 255
Logical Address: 27595, Physical Address: 27595, Value: 255
Logical Address: 33025, Physical Address: 33025, Value: 255
Logical Address: 53061, Physical Address: 53061, Value: 255
Logical Address: 14295, Physical Address: 14295, Value: 255
Logical Address: 43260, Physical Address: 43260, Value: 255
Logical Address: 11760, Physical Address: 11760, Value: 255
Logical Address: 22260, Physical Address: 22260, Value: 255
Logical Address: 18760, Physical Address: 18760, Value: 255
Logical Address: 14125, Physical Address: 14125, Value: 255
Logical Address: 59361, Physical Address: 59361, Value: 255
Logical Address: 12195, Physical Address: 12195, Value: 255
Logical Address: 43960, Physical Address: 43960, Value: 255
Logical Address: 5725, Physical Address: 5725, Value: 255
Logical Address: 62161, Physical Address: 62161, Value: 255
Logical Address: 5460, Physical Address: 5460, Value: 255
Logical Address: 24360, Physical Address: 24360, Value: 255
Logical Address: 18060, Physical Address: 18060, Value: 255
Logical Address: 20160, Physical Address: 20160, Value: 255
Logical Address: 19460, Physical Address: 19460, Value: 255
Logical Address: 25495, Physical Address: 25495, Value: 255
Logical Address: 51130, Physical Address: 51130, Value: 255
Logical Address: 64430, Physical Address: 64430, Value: 255
Logical Address: 54195, Physical Address: 54195, Value: 255
Logical Address: 29960, Physical Address: 29960, Value: 255
Logical Address: 21995, Physical Address: 21995, Value: 255
Logical Address: 46495, Physical Address: 46495, Value: 255
Logical Address: 26725, Physical Address: 26725, Value: 255
Logical Address: 55161, Physical Address: 55161, Value: 255
Logical Address: 13595, Physical Address: 13595, Value: 255
Logical Address: 49295, Physical Address: 49295, Value: 255
Logical Address: 37395, Physical Address: 37395, Value: 255
Logical Address: 35560, Physical Address: 35560, Value: 255
Logical Address: 8525, Physical Address: 8525, Value: 255
Logical Address: 7295, Physical Address: 7295, Value: 255
Logical Address: 51395, Physical Address: 51395, Value: 255
Logical Address: 36695, Physical Address: 36695, Value: 255
Logical Address: 41595, Physical Address: 41595, Value: 255
Logical Address: 34160, Physical Address: 34160, Value: 255
Logical Address: 20595, Physical Address: 20595, Value: 255
Logical Address: 41160, Physical Address: 41160, Value: 255
Logical Address: 12460, Physical Address: 12460, Value: 255
Logical Address: 16225, Physical Address: 16225, Value: 255
Logical Address: 58661, Physical Address: 58661, Value: 255
Logical Address: 825, Physical Address: 825, Value: 255
Logical Address: 4060, Physical Address: 4060, Value: 255
Logical Address: 19025, Physical Address: 19025, Value: 255
Logical Address: 3795, Physical Address: 3795, Value: 255
Logical Address: 46760, Physical Address: 46760, Value: 255
Logical Address: 16395, Physical Address: 16395, Value: 255
Logical Address: 42560, Physical Address: 42560, Value: 255
Logical Address: 17795, Physical Address: 17795, Value: 255
Logical Address: 47895, Physical Address: 47895, Value: 255
Logical Address: 32060, Physical Address: 32060, Value: 255
Logical Address: 21295, Physical Address: 21295, Value: 255
Logical Address: 52530, Physical Address: 52530, Value: 255
Logical Address: 52360, Physical Address: 52360, Value: 255
Logical Address: 2925, Physical Address: 2925, Value: 255
Logical Address: 3360, Physical Address: 3360, Value: 255
Logical Address: 25060, Physical Address: 25060, Value: 255
Logical Address: 12025, Physical Address: 12025, Value: 255
Logical Address: 60061, Physical Address: 60061, Value: 255
Logical Address: 6160, Physical Address: 6160, Value: 255
Logical Address: 18325, Physical Address: 18325, Value: 255
Logical Address: 57961, Physical Address: 57961, Value: 255
Logical Address: 6860, Physical Address: 6860, Value: 255
Logical Address: 29695, Physical Address: 29695, Value: 255
Logical Address: 32325, Physical Address: 32325, Value: 255
Logical Address: 59096, Physical Address: 59096, Value: 255
Logical Address: 22525, Physical Address: 22525, Value: 255
Logical Address: 56561, Physical Address: 56561, Value: 255
Logical Address: 1525, Physical Address: 1525, Value: 255
Logical Address: 63561, Physical Address: 63561, Value: 255
Logical Address: 10795, Physical Address: 10795, Value: 255
Logical Address: 56030, Physical Address: 56030, Value: 255
Logical Address: 56995, Physical Address: 56995, Value: 255
Logical Address: 23225, Physical Address: 23225, Value: 255
Logical Address: 2395, Physical Address: 2395, Value: 255
Logical Address: 58830, Physical Address: 58830, Value: 255
Logical Address: 50260, Physical Address: 50260, Value: 255
Logical Address: 3625, Physical Address: 3625, Value: 255
Logical Address: 62861, Physical Address: 62861, Value: 255
Logical Address: 64961, Physical Address: 64961, Value: 255
Logical Address: 64261, Physical Address: 64261, Value: 255
Logical Address: 4760, Physical Address: 4760, Value: 255
Logical Address: 30395, Physical Address: 30395, Value: 255
Logical Address: 43695, Physical Address: 43695, Value: 255
Logical Address: 33460, Physical Address: 33460, Value: 255
Logical Address: 9225, Physical Address: 9225, Value: 255
Logical Address: 1260, Physical Address: 1260, Value: 255
Logical Address: 25760, Physical Address: 25760, Value: 255
Logical Address: 23395, Physical Address: 23395, Value: 255
Logical Address: 51830, Physical Address: 51830, Value: 255
Logical Address: 58395, Physical Address: 58395, Value: 255
Logical Address: 28560, Physical Address: 28560, Value: 255
Logical Address: 16660, Physical Address: 16660, Value: 255
Logical Address: 14825, Physical Address: 14825, Value: 255
Logical Address: 5195, Physical Address: 5195, Value: 255
Logical Address: 52095, Physical Address: 52095, Value: 255
Logical Address: 30660, Physical Address: 30660, Value: 255
Logical Address: 15960, Physical Address: 15960, Value: 255
Logical Address: 20860, Physical Address: 20860, Value: 255
Logical Address: 13425, Physical Address: 13425, Value: 255
Logical Address: 65396, Physical Address: 65396, Value: 255
Logical Address: 20425, Physical Address: 20425, Value: 255
Logical Address: 57261, Physical Address: 57261, Value: 255
Logical Address: 12895, Physical Address: 12895, Value: 255
Logical Address: 55330, Physical Address: 55330, Value: 255
Logical Address: 63030, Physical Address: 63030, Value: 255
Logical Address: 48860, Physical Address: 48860, Value: 255
Logical Address: 15695, Physical Address: 15695, Value: 255
Logical Address: 48595, Physical Address: 48595, Value: 255
Logical Address: 26025, Physical Address: 26025, Value: 255
Logical Address: 61196, Physical Address: 61196, Value: 255
Logical Address: 21825, Physical Address: 21825, Value: 255
Logical Address: 62596, Physical Address: 62596, Value: 255
Logical Address: 27160, Physical Address: 27160, Value: 255
Logical Address: 11325, Physical Address: 11325, Value: 255
Logical Address: 560, Physical Address: 560, Value: 255
Logical Address: 31795, Physical Address: 31795, Value: 255
Logical Address: 31625, Physical Address: 31625, Value: 255
Logical Address: 65131, Physical Address: 65131, Value: 255
Logical Address: 48160, Physical Address: 48160, Value: 255
Logical Address: 4325, Physical Address: 4325, Value: 255
Logical Address: 8695, Physical Address: 8695, Value: 255
Logical Address: 56730, Physical Address: 56730, Value: 255
Logical Address: 50960, Physical Address: 50960, Value: 255
Logical Address: 14995, Physical Address: 14995, Value: 255
Logical Address: 54630, Physical Address: 54630, Value: 255
Logical Address: 51660, Physical Address: 51660, Value: 255
Logical Address: 8960, Physical Address: 8960, Value: 255
Logical Address: 28995, Physical Address: 28995, Value: 255
Logical Address: 38360, Physical Address: 38360, Value: 255
Logical Address: 19195, Physical Address: 19195, Value: 255
Logical Address: 53230, Physical Address: 53230, Value: 255
Logical Address: 63730, Physical Address: 63730, Value: 255
Logical Address: 60230, Physical Address: 60230, Value: 255
Logical Address: 55595, Physical Address: 55595, Value: 255
Logical Address: 35295, Physical Address: 35295, Value: 255
Logical Address: 36260, Physical Address: 36260, Value: 255
Logical Address: 19895, Physical Address: 19895, Value: 255
Logical Address: 47195, Physical Address: 47195, Value: 255
Logical Address: 38095, Physical Address: 38095, Value: 255
Logical Address: 29525, Physical Address: 29525, Value: 255
Logical Address: 295, Physical Address: 295, Value: 255
Logical Address: 59530, Physical Address: 59530, Value: 255
Logical Address: 61630, Physical Address: 61630, Value: 255
Logical Address: 60930, Physical Address: 60930, Value: 255
Logical Address: 49560, Physical Address: 49560, Value: 255
Logical Address: 9660, Physical Address: 9660, Value: 255
Logical Address: 22960, Physical Address: 22960, Value: 255
Logical Address: 12725, Physical Address: 12725, Value: 255
Logical Address: 5895, Physical Address: 5895, Value: 255
Logical Address: 46060, Physical Address: 46060, Value: 255
Logical Address: 5025, Physical Address: 5025, Value: 255
Logical Address: 2660, Physical Address: 2660, Value: 255
Logical Address: 31095, Physical Address: 31095, Value: 255
Logical Address: 37660, Physical Address: 37660, Value: 255
Logical Address: 7825, Physical Address: 7825, Value: 255
Logical Address: 61461, Physical Address: 61461, Value: 255
Logical Address: 11495, Physical Address: 11495, Value: 255
Logical Address: 49995, Physical Address: 49995, Value: 255
Logical Address: 31360, Physical Address: 31360, Value: 255
Logical Address: 9925, Physical Address: 9925, Value: 255
Logical Address: 60761, Physical Address: 60761, Value: 255
Logical Address: 125, Physical Address: 125, Value: 255
Logical Address: 10095, Physical Address: 10095, Value: 255
Logical Address: 44660, Physical Address: 44660, Value: 255
Logical Address: 17095, Physical Address: 17095, Value: 255
Logical Address: 53930, Physical Address: 53930, Value: 255
Logical Address: 57695, Physical Address: 57695, Value: 255
Logical Address: 34595, Physical Address: 34595, Value: 255
Logical Address: 42295, Physical Address: 42295, Value: 255
Logical Address: 28125, Physical Address: 28125, Value: 255
Logical Address: 60496, Physical Address: 60496, Value: 255
Logical Address: 27860, Physical Address: 27860, Value: 255
Logical Address: 22695, Physical Address: 22695, Value: 255
Logical Address: 40460, Physical Address: 40460, Value: 255
Logical Address: 18495, Physical Address: 18495, Value: 255
Logical Address: 41860, Physical Address: 41860, Value: 255
Logical Address: 6425, Physical Address: 6425, Value: 255
Logical Address: 7995, Physical Address: 7995, Value: 255
Logical Address: 45360, Physical Address: 45360, Value: 255
Logical Address: 11060, Physical Address: 11060, Value: 255
Logical Address: 28295, Physical Address: 28295, Value: 255
Logical Address: 44395, Physical Address: 44395, Value: 255
Logical Address: 27425, Physical Address: 27425, Value: 255
Logical Address: 995, Physical Address: 995, Value: 255
Logical Address: 53495, Physical Address: 53495, Value: 255
Logical Address: 35995, Physical Address: 35995, Value: 255
Logical Address: 30225, Physical Address: 30225, Value: 255
Logical Address: 59796, Physical Address: 59796, Value: 255
Logical Address: 33895, Physical Address: 33895, Value: 255
Logical Address: 30925, Physical Address: 30925, Value: 255
Logical Address: 53761, Physical Address: 53761, Value: 255
Logical Address: 8260, Physical Address: 8260, Value: 255
Logical Address: 17625, Physical Address: 17625, Value: 255
Logical Address: 63996, Physical Address: 63996, Value: 255
Logical Address: 32495, Physical Address: 32495, Value: 255
Logical Address: 42995, Physical Address: 42995, Value: 255
Logical Address: 39495, Physical Address: 39495, Value: 255
Logical Address: 34860, Physical Address: 34860, Value: 255
Logical Address: 14560, Physical Address: 14560, Value: 255
Logical Address: 15525, Physical Address: 15525, Value: 255
Logical Address: 64696, Physical Address: 64696, Value: 255
Logical Address: 26460, Physical Address: 26460, Value: 255
Logical Address: 17360, Physical Address: 17360, Value: 255
Logical Address: 26195, Physical Address: 26195, Value: 255
Logical Address: 45095, Physical Address: 45095, Value: 255
Logical Address: 38795, Physical Address: 38795, Value: 255
Logical Address: 40895, Physical Address: 40895, Value: 255
Logical Address: 40195, Physical Address: 40195, Value: 255
Logical Address: 28825, Physical Address: 28825, Value: 255
Logical Address: 54461, Physical Address: 54461, Value: 255
Logical Address: 2225, Physical Address: 2225, Value: 255
Logical Address: 9395, Physical Address: 9395, Value: 255
Logical Address: 50695, Physical Address: 50695, Value: 255
Logical Address: 25325, Physical Address: 25325, Value: 255
Logical Address: 1695, Physical Address: 1695, Value: 255
Logical Address: 47460, Physical Address: 47460, Value: 255
Logical Address: 10360, Physical Address: 10360, Value: 255
Logical Address: 16925, Physical Address: 16925, Value: 255
Logical Address: 4495, Physical Address: 4495, Value: 255
Logical Address: 58130, Physical Address: 58130, Value: 255
Logical Address: 56295, Physical Address: 56295, Value: 255
Logical Address: 29260, Physical Address: 29260, Value: 255
Logical Address: 10625, Physical Address: 10625, Value: 255
Logical Address: 6595, Physical Address: 6595, Value: 255
Logical Address: 57430, Physical Address: 57430, Value: 255
Logical Address: 62330, Physical Address: 62330, Value: 255
Logical Address: 54895, Physical Address: 54895, Value: 255
Logical Address: 23925, Physical Address: 23925, Value: 255
Logical Address: 61896, Physical Address: 61896, Value: 255
Logical Address: 33195, Physical Address: 33195, Value: 255
Logical Address: 36960, Physical Address: 36960, Value: 255
Logical Address: 13860, Physical Address: 13860, Value: 255
Logical Address: 21560, Physical Address: 21560, Value: 255
Logical Address: 24795, Physical Address: 24795, Value: 255
Logical Address: 39760, Physical Address: 39760, Value: 255
Logical Address: 7125, Physical Address: 7125, Value: 255
Logical Address: 1960, Physical Address: 1960, Value: 255
Logical Address: 19725, Physical Address: 19725, Value: 255
Logical Address: 63296, Physical Address: 63296, Value: 255
Logical Address: 21125, Physical Address: 21125, Value: 255
Logical Address: 3095, Physical Address: 3095, Value: 255
Logical Address: 52795, Physical Address: 52795, Value: 255
Logical Address: 24625, Physical Address: 24625, Value: 255
Logical Address: 55861, Physical Address: 55861, Value: 255
Logical Address: 7560, Physical Address: 7560, Value: 255
Logical Address: 23660, Physical Address: 23660, Value: 255
Logical Address: 24095, Physical Address: 24095, Value: 255
Logical Address: 45795, Physical Address: 45795, Value: 255
Logical Address: 32760, Physical Address: 32760, Value: 255
Logical Address: 15260, Physical Address: 15260, Value: 255
Logical Address: 26895, Physical Address: 26895, Value: 255
Logical Address: 39060, Physical Address: 39060, Value: 255
Logical Address: 13160, Physical Address: 13160, Value: 255
Logical Address: 27595, Physical Address: 27595, Value: 255
Logical Address: 33025, Physical Address: 33025, Value: 255
Logical Address: 53061, Physical Address: 53061, Value: 255
Logical Address: 14295, Physical Address: 14295, Value: 255
Logical Address: 43260, Physical Address: 43260, Value: 255
Logical Address: 11760, Physical Address: 11760, Value: 255
Logical Address: 22260, Physical Address: 22260, Value: 255
Logical Address: 18760, Physical Address: 18760, Value: 255
Logical Address: 14125, Physical Address: 14125, Value: 255
Logical Address: 59361, Physical Address: 59361, Value: 255
Logical Address: 12195, Physical Address: 12195, Value: 255
Logical Address: 43960, Physical Address: 43960, Value: 255
Logical Address: 5725, Physical Address: 5725, Value: 255
Logical Address: 62161, Physical Address: 62161, Value: 255
Logical Address: 5460, Physical Address: 5460, Value: 255
Logical Address: 24360, Physical Address: 24360, Value: 255
Logical Address: 18060, Physical Address: 18060, Value: 255
Logical Address: 20160, Physical Address: 20160, Value: 255
Logical Address: 19460, Physical Address: 19460, Value: 255
Logical Address: 25495, Physical Address: 25495, Value: 255
Logical Address: 51130, Physical Address: 51130, Value: 255
Logical Address: 64430, Physical Address: 64430, Value: 255
Logical Address: 54195, Physical Address: 54195, Value: 255
Logical Address: 29960, Physical Address: 29960, Value: 255
Logical Address: 21995, Physical Address: 21995, Value: 255
Logical Address: 46495, Physical Address: 46495, Value: 255
Logical Address: 26725, Physical Address: 26725, Value: 255
Logical Address: 55161, Physical Address: 55161, Value: 255
Logical Address: 13595, Physical Address: 13595, Value: 255
Logical Address: 49295, Physical Address: 49295, Value: 255
Logical Address: 37395, Physical Address: 37395, Value: 255
Logical Address: 35560, Physical Address: 35560, Value: 255
Logical Address: 8525, Physical Address: 8525, Value: 255
Logical Address: 7295, Physical Address: 7295, Value: 255
Logical Address: 51395, Physical Address: 51395, Value: 255
Logical Address: 36695, Physical Address: 36695, Value: 255
Logical Address: 41595, Physical Address: 41595, Value: 255
Logical Address: 34160, Physical Address: 34160, Value: 255
Logical Address: 20595, Physical Address: 20595, Value: 255
Logical Address: 41160, Physical Address: 41160, Value: 255
Logical Address: 12460, Physical Address: 12460, Value: 255
Logical Address: 16225, Physical Address: 16225, Value: 255
Logical Address: 4060, Physical Address: 4060, Value: 255
Logical Address: 19025, Physical Address: 19025, Value: 255
Logical Address: 3795, Physical Address: 3795, Value: 255
Logical Address: 46760, Physical Address: 46760, Value: 255
Logical Address: 16395, Physical Address: 16395, Value: 255
Logical Address: 42560, Physical Address: 42560, Value: 255
Logical Address: 17795, Physical Address: 17795, Value: 255
Logical Address: 47895, Physical Address: 47895, Value: 255
Logical Address: 32060, Physical Address: 32060, Value: 255
Logical Address: 21295, Physical Address: 21295, Value: 255
Logical Address: 52530, Physical Address: 52530, Value: 255
Logical Address: 52360, Physical Address: 52360, Value: 255
Logical Address: 2925, Physical Address: 2925, Value: 255
Logical Address: 3360, Physical Address: 3360, Value: 255
Logical Address: 25060, Physical Address: 25060, Value: 255
Logical Address: 12025, Physical Address: 12025, Value: 255
Logical Address: 60061, Physical Address: 60061, Value: 255
Logical Address: 6160, Physical Address: 6160, Value: 255
Logical Address: 18325, Physical Address: 18325, Value: 255
Logical Address: 57961, Physical Address: 57961, Value: 255
Logical Address: 6860, Physical Address: 6860, Value: 255
Logical Address: 29695, Physical Address: 29695, Value: 255
Logical Address: 32325, Physical Address: 32325, Value: 255
Logical Address: 59096, Physical Address: 59096, Value: 255
Logical Address: 22525, Physical Address: 22525, Value: 255
Logical Address: 56561, Physical Address: 56561, Value: 255
Logical Address: 1525, Physical Address: 1525, Value: 255
Logical Address: 63561, Physical Address: 63561, Value: 255
Logical Address: 10795, Physical Address: 10795, Value: 255
Logical Address: 56030, Physical Address: 56030, Value: 255
Logical Address: 56995, Physical Address: 56995, Value: 255
Logical Address: 23225, Physical Address: 23225, Value: 255
Logical Address: 2395, Physical Address: 2395, Value: 255
Logical Address: 58830, Physical Address: 58830, Value: 255
Logical Address: 50260, Physical Address: 50260, Value: 255
Logical Address: 3625, Physical Address: 3625, Value: 255
Logical Address: 62861, Physical Address: 62861, Value: 255
Logical Address: 64961, Physical Address: 64961, Value: 255
Logical Address: 64261, Physical Address: 64261, Value: 255
Logical Address: 4760, Physical Address: 4760, Value: 255
Logical Address: 30395, Physical Address: 30395, Value: 255
Logical Address: 43695, Physical Address: 43695, Value: 255
Logical Address: 33460, Physical Address: 33460, Value: 255
Logical Address: 9225, Physical Address: 9225, Value: 255
Logical Address: 1260, Physical Address: 1260, Value: 255
Logical Address: 25760, Physical Address: 25760, Value: 255
Logical Address: 23395, Physical Address: 23395, Value: 255
Logical Address: 51830, Physical Address: 51830, Value: 255
Logical Address: 58395, Physical Address: 58395, Value: 255
Logical Address: 28560, Physical Address: 28560, Value: 255
Logical Address: 16660, Physical Address: 16660, Value: 255
Logical Address: 14825, Physical Address: 14825, Value: 255
Logical Address: 5195, Physical Address: 5195, Value: 255
Logical Address: 52095, Physical Address: 52095, Value: 255
Logical Address: 30660, Physical Address: 30660, Value: 255
Logical Address: 15960, Physical Address: 15960, Value: 255
Logical Address: 20860, Physical Address: 20860, Value: 255
Logical Address: 13425, Physical Address: 13425, Value: 255
Logical Address: 65396, Physical Address: 65396, Value: 255
Logical Address: 20425, Physical Address: 20425, Value: 255
Logical Address: 57261, Physical Address: 57261, Value: 255
Logical Address: 12895, Physical Address: 12895, Value: 255
Logical Address: 55330, Physical Address: 55330, Value: 255
Logical Address: 63030, Physical Address: 63030, Value: 255
Logical Address: 48860, Physical Address: 48860, Value: 255
Logical Address: 15695, Physical Address: 15695, Value: 255
Logical Address: 48595, Physical Address: 48595, Value: 255
Logical Address: 26025, Physical Address: 26025, Value: 255
Logical Address: 61196, Physical Address: 61196, Value: 255
Logical Address: 21825, Physical Address: 21825, Value: 255
Logical Address: 62596, Physical Address: 62596, Value: 255
Logical Address: 27160, Physical Address: 27160, Value: 255
Logical Address: 11325, Physical Address: 11325, Value: 255
Logical Address: 560, Physical Address: 560, Value: 255
Logical Address: 31795, Physical Address: 31795, Value: 255
Logical Address: 31625, Physical Address: 31625, Value: 255
Logical Address: 65131, Physical Address: 65131, Value: 255
Logical Address: 48160, Physical Address: 48160, Value: 255
Logical Address: 4325, Physical Address: 4325, Value: 255
Logical Address: 8695, Physical Address: 8695, Value: 255
Logical Address: 56730, Physical Address: 56730, Value: 255
Logical Address: 50960, Physical Address: 50960, Value: 255
Logical Address: 14995, Physical Address: 14995, Value: 255
Logical Address: 54630, Physical Address: 54630, Value: 255
Logical Address: 51660, Physical Address: 51660, Value: 255
Logical Address: 8960, Physical Address: 8960, Value: 255
Logical Address: 28995, Physical Address: 28995, Value: 255
Logical Address: 38360, Physical Address: 38360, Value: 255
Logical Address: 19195, Physical Address: 19195, Value: 255
Logical Address: 53230, Physical Address: 53230, Value: 255
Logical Address: 63730, Physical Address: 63730, Value: 255
Logical Address: 60230, Physical Address: 60230, Value: 255
Logical Address: 55595, Physical Address: 55595, Value: 255
Logical Address: 35295, Physical Address: 35295, Value: 255
Logical Address: 36260, Physical Address: 36260, Value: 255
Logical Address: 19895, Physical Address: 19895, Value: 255
Logical Address: 47195, Physical Address: 47195, Value: 255
Logical Address: 38095, Physical Address: 38095, Value: 255
Logical Address: 29525, Physical Address: 29525, Value: 255
Logical Address: 295, Physical Address: 295, Value: 255
Logical Address: 59530, Physical Address: 59530, Value: 255
Logical Address: 61630, Physical Address: 61630, Value: 255
Logical Address: 60930, Physical Address: 60930, Value: 255
Logical Address: 49560, Physical Address: 49560, Value: 255
Logical Address: 9660, Physical Address: 9660, Value: 255
Logical Address: 22960, Physical Address: 22960, Value: 255
Logical Address: 12725, Physical Address: 12725, Value: 255
Logical Address: 5895, Physical Address: 5895, Value: 255
Logical Address: 46060, Physical Address: 46060, Value: 255
Logical Address: 5025, Physical Address: 5025, Value: 255
Logical Address: 2660, Physical Address: 2660, Value: 255
Logical Address: 31095, Physical Address: 31095, Value: 255
Logical Address: 37660, Physical Address: 37660, Value: 255
Logical Address: 7825, Physical Address: 7825, Value: 255
Logical Address: 61461, Physical Address: 61461, Value: 255
Logical Address: 11495, Physical Address: 11495, Value: 255
Logical Address: 49995, Physical Address: 49995, Value: 255
Logical Address: 31360, Physical Address: 31360, Value: 255
Logical Address: 9925, Physical Address: 9925, Value: 255
Logical Address: 60761, Physical Address: 60761, Value: 255
Logical Address: 125, Physical Address: 125, Value: 255
Logical Address: 10095, Physical Address: 10095, Value: 255
Logical Address: 44660, Physical Address: 44660, Value: 255
Logical Address: 17095, Physical Address: 17095, Value: 255
Logical Address: 53930, Physical Address: 53930, Value: 255
Logical Address: 57695, Physical Address: 57695, Value: 255
Logical Address: 34595, Physical Address: 34595, Value: 255
Logical Address: 42295, Physical Address: 42295, Value: 255
Logical Address: 28125, Physical Address: 28125, Value: 255
Logical Address: 60496, Physical Address: 60496, Value: 255
Logical Address: 27860, Physical Address: 27860, Value: 255
Logical Address: 22695, Physical Address: 22695, Value: 255
Logical Address: 40460, Physical Address: 40460, Value: 255
Logical Address: 18495, Physical Address: 18495, Value: 255
Logical Address: 41860, Physical Address: 41860, Value: 255
Logical Address: 6425, Physical Address: 6425, Value: 255
Logical Address: 7995, Physical Address: 7995, Value: 255
Logical Address: 45360, Physical Address: 45360, Value: 255
Logical Address: 11060, Physical Address: 11060, Value: 255
Logical Address: 28295, Physical Address: 28295, Value: 255
Logical Address: 44395, Physical Address: 44395, Value: 255
Logical Address: 27425, Physical Address: 27425, Value: 255
Logical Address: 995, Physical Address: 995, Value: 255
Logical Address: 53495, Physical Address: 53495, Value: 255
Logical Address: 35995, Physical Address: 35995, Value: 255
Logical Address: 30225, Physical Address: 30225, Value: 255
Logical Address: 59796, Physical Address: 59796, Value: 255
Logical Address: 33895, Physical Address: 33895, Value: 255
Logical Address: 30925, Physical Address: 30925, Value: 255
Logical Address: 53761, Physical Address: 53761, Value: 255
Logical Address: 8260, Physical Address: 8260, Value: 255
Logical Address: 17625, Physical Address: 17625, Value: 255
Logical Address: 63996, Physical Address: 63996, Value: 255
Logical Address: 32495, Physical Address: 32495, Value: 255
Logical Address: 42995, Physical Address: 42995, Value: 255
Logical Address: 39495, Physical Address: 39495, Value: 255
Logical Address: 34860, Physical Address: 34860, Value: 255
Logical Address: 14560, Physical Address: 14560, Value: 255
Logical Address: 15525, Physical Address: 15525, Value: 255
Logical Address: 64696, Physical Address: 64696, Value: 255
Logical Address: 26460, Physical Address: 26460, Value: 255
Logical Address: 17360, Physical Address: 17360, Value: 255
Logical Address: 26195, Physical Address: 26195, Value: 255
Logical Address: 45095, Physical Address: 45095, Value: 255
Logical Address: 38795, Physical Address: 38795, Value: 255
Logical Address: 40895, Physical Address: 40895, Value: 255
Logical Address: 40195, Physical Address: 40195, Value: 255
Logical Address: 28825, Physical Address: 28825, Value: 255
Logical Address: 54461, Physical Address: 54461, Value: 255
Logical Address: 2225, Physical Address: 2225, Value: 255
Logical Address: 9395, Physical Address: 9395, Value: 255
Logical Address: 50695, Physical Address: 50695, Value: 255
Logical Address: 25325, Physical Address: 25325, Value: 255
Logical Address: 1695, Physical Address: 1695, Value: 255
Logical Address: 47460, Physical Address: 47460, Value: 255
Logical Address: 10360, Physical Address: 10360, Value: 255
Logical Address: 16925, Physical Address: 16925, Value: 255
Logical Address: 4495, Physical Address: 4495, Value: 255
Logical Address: 58130, Physical Address: 58130, Value: 255
Logical Address: 56295, Physical Address: 56295, Value: 255
Logical Address: 29260, Physical Address: 29260, Value: 255
Logical Address: 10625, Physical Address: 10625, Value: 255
Logical Address: 6595, Physical Address: 6595, Value: 255
Logical Address: 57430, Physical Address: 57430, Value: 255
Logical Address: 62330, Physical Address: 62330, Value: 255
Logical Address: 54895, Physical Address: 54895, Value: 255
Logical Address: 23925, Physical Address: 23925, Value: 255
Logical Address: 61896, Physical Address: 61896, Value: 255
Logical Address: 33195, Physical Address: 33195, Value: 255
Logical Address: 36960, Physical Address: 36960, Value: 255
Logical Address: 13860, Physical Address: 13860, Value: 255
Logical Address: 21560, Physical Address: 21560, Value: 255
Logical Address: 24795, Physical Address: 24795, Value: 255
Logical Address: 39760, Physical Address: 39760, Value: 255
Logical Address: 7125, Physical Address: 7125, Value: 255
Logical Address: 1960, Physical Address: 1960, Value: 255
Logical Address: 19725, Physical Address: 19725, Value: 255
Logical Address: 63296, Physical Address: 63296, Value: 255
Logical Address: 21125, Physical Address: 21125, Value: 255
Logical Address: 3095, Physical Address: 3095, Value: 255
Logical Address: 52795, Physical Address: 52795, Value: 255
Logical Address: 24625, Physical Address: 24625, Value: 255
Logical Address: 55861, Physical Address: 55861, Value: 255
Logical Address: 7560, Physical Address: 7560, Value: 255
Logical Address: 23660, Physical Address: 23660, Value: 255
Logical Address: 24095, Physical Address: 24095, Value: 255
Logical Address: 45795, Physical Address: 45795, Value: 255
Logical Address: 32760, Physical Address: 32760, Value: 255
Logical Address: 15260, Physical Address: 15260, Value: 255
Logical Address: 26895, Physical Address: 26895, Value: 255
Logical Address: 39060, Physical Address: 39060, Value: 255
Logical Address: 13160, Physical Address: 13160, Value: 255
Logical Address: 27595, Physical Address: 27595, Value: 255
Logical Address: 33025, Physical Address: 33025, Value: 255
Logical Address: 53061, Physical Address: 53061, Value: 255
Logical Address: 14295, Physical Address: 14295, Value: 255
Logical Address: 43260, Physical Address: 43260, Value: 255
Logical Address: 11760, Physical Address: 11760, Value: 255
Logical Address: 22260, Physical Address: 22260, Value: 255
Logical Address: 18760, Physical Address: 18760, Value: 255
Logical Address: 14125, Physical Address: 14125, Value: 255
Logical Address: 59361, Physical Address: 59361, Value: 255
Logical Address: 12195, Physical Address: 12195, Value: 255
Logical Address: 43960, Physical Address: 43960, Value: 255
Logical Address: 5725, Physical Address: 5725, Value: 255
Logical Address: 62161, Physical Address: 62161, Value: 255
Logical Address: 5460, Physical Address: 5460, Value: 255
Logical Address: 24360, Physical Address: 24360, Value: 255
Logical Address: 18060, Physical Address: 18060, Value: 255
Logical Address: 20160, Physical Address: 20160, Value: 255
Logical Address: 19460, Physical Address: 19460, Value: 255
Logical Address: 25495, Physical Address: 25495, Value: 255
Logical Address: 51130, Physical Address: 51130, Value: 255
Logical Address: 64430, Physical Address: 64430, Value: 255
Logical Address: 54195, Physical Address: 54195, Value: 255
Logical Address: 29960, Physical Address: 29960, Value: 255
Logical Address: 21995, Physical Address: 21995, Value: 255
Logical Address: 46495, Physical Address: 46495, Value: 255
Logical Address: 26725, Physical Address: 26725, Value: 255
Logical Address: 55161, Physical Address: 55161, Value: 255
Logical Address: 13595, Physical Address: 13595, Value: 255
Logical Address: 49295, Physical Address: 49295, Value: 255
Logical Address: 37395, Physical Address: 37395, Value: 255
Logical Address: 35560, Physical Address: 35560, Value: 255
Logical Address: 8525, Physical Address: 8525, Value: 255
Logical Address: 7295, Physical Address: 7295, Value: 255
Logical Address: 51395, Physical Address: 51395, Value: 255
Logical Address: 36695, Physical Address: 36695, Value: 255
Logical Address: 41595, Physical Address: 41595, Value: 255
Logical Address: 34160, Physical Address: 34160, Value: 255
Logical Address: 20595, Physical Address: 20595, Value: 255
Logical Address: 41160, Physical Address: 41160, Value: 255
Logical Address: 12460, Physical Address: 12460, Value: 255
Logical Address: 16225, Physical Address: 16225, Value: 255
Logical Address: 58661, Physical Address: 58661, Value: 255
Logical Address: 825, Physical Address: 825, Value: 255
Logical Address: 4060, Physical Address: 4060, Value: 255
Logical Address: 19025, Physical Address: 19025, Value: 255
Logical Address: 3795, Physical Address: 3795, Value: 255
Logical Address: 46760, Physical Address: 46760, Value: 255
Logical Address: 16395, Physical Address: 16395, Value: 255
Logical Address: 42560, Physical Address: 42560, Value: 255
Logical Address: 17795, Physical Address: 17795, Value: 255
Logical Address: 47895, Physical Address: 47895, Value: 255
Logical Address: 32060, Physical Address: 32060, Value: 255
Logical Address: 21295, Physical Address: 21295, Value: 255
Logical Address: 52530, Physical Address: 52530, Value: 255
Logical Address: 52360, Physical Address: 52360, Value: 255
Logical Address: 2925, Physical Address: 2925, Value: 255
Logical Address: 3360, Physical Address: 3360, Value: 255
Logical Address: 25060, Physical Address: 25060, Value: 255
Logical Address: 12025, Physical Address: 12025, Value: 255
Logical Address: 60061, Physical Address: 60061, Value: 255
Logical Address: 6160, Physical Address: 6160, Value: 255
Logical Address: 18325, Physical Address: 18325, Value: 255
Logical Address: 57961, Physical Address: 57961, Value: 255
Logical Address: 6860, Physical Address: 6860, Value: 255
Logical Address: 29695, Physical Address: 29695, Value: 255
Logical Address: 32325, Physical Address: 32325, Value: 255
Logical Address: 59096, Physical Address: 59096, Value: 255
Logical Address: 22525, Physical Address: 22525, Value: 255
Logical Address: 56561, Physical Address: 56561, Value: 255
Logical Address: 1525, Physical Address: 1525, Value: 255
Logical Address: 63561, Physical Address: 63561, Value: 255
Logical Address: 10795, Physical Address: 10795, Value: 255
Logical Address: 56030, Physical Address: 56030, Value: 255
Logical Address: 56995, Physical Address: 56995, Value: 255
Logical Address: 23225, Physical Address: 23225, Value: 255
Logical Address: 2395, Physical Address: 2395, Value: 255
Logical Address: 58830, Physical Address: 58830, Value: 255
Logical Address: 50260, Physical Address: 50260, Value: 255
Logical Address: 3625, Physical Address: 3625, Value: 255
Logical Address: 62861, Physical Address: 62861, Value: 255
Logical Address: 64961, Physical Address: 64961, Value: 255
Logical Address: 64261, Physical Address: 64261, Value: 255
Logical Address: 4760, Physical Address: 4760, Value: 255
Logical Address: 30395, Physical Address: 30395, Value: 255
Logical Address: 43695, Physical Address: 43695, Value: 255
Logical Address: 33460, Physical Address: 33460, Value: 255
Logical Address: 9225, Physical Address: 9225, Value: 255
Logical Address: 1260, Physical Address: 1260, Value: 255
Logical Address: 25760, Physical Address: 25760, Value: 255
Logical Address: 23395, Physical Address: 23395, Value: 255
Logical Address: 51830, Physical Address: 51830, Value: 255
Logical Address: 58395, Physical Address: 58395, Value: 255
Logical Address: 28560, Physical Address: 28560, Value: 255
Logical Address: 16660, Physical Address: 16660, Value: 255
Logical Address: 14825, Physical Address: 14825, Value: 255
Logical Address: 5195, Physical Address: 5195, Value: 255
Logical Address: 52095, Physical Address: 52095, Value: 255
Logical Address: 30660, Physical Address: 30660, Value: 255
Logical Address: 15960, Physical Address: 15960, Value: 255
Logical Address: 20860, Physical Address: 20860, Value: 255
Logical Address: 13425, Physical Address: 13425, Value: 255
Logical Address: 65396, Physical Address: 65396, Value: 255
Logical Address: 20425, Physical Address: 20425, Value: 255
Logical Address: 57261, Physical Address: 57261, Value: 255
Logical Address: 12895, Physical Address: 12895, Value: 255
Logical Address: 55330, Physical Address: 55330, Value: 255
Logical Address: 63030, Physical Address: 63030, Value: 255
Logical Address: 48860, Physical Address: 48860, Value: 255
Logical Address: 15695, Physical Address: 15695, Value: 255
Logical Address: 48595, Physical Address: 48595, Value: 255
Logical Address: 26025, Physical Address: 26025, Value: 255
Logical Address: 61196, Physical Address: 61196, Value: 255
Logical Address: 21825, Physical Address: 21825, Value: 255
Logical Address: 62596, Physical Address: 62596, Value: 255
Logical Address: 27160, Physical Address: 27160, Value: 255
Logical Address: 11325, Physical Address: 11325, Value: 255
Logical Address: 560, Physical Address: 560, Value: 255
Logical Address: 31795, Physical Address: 31795, Value: 255
Logical Address: 31625, Physical Address: 31625, Value: 255
Logical Address: 65131, Physical Address: 65131, Value: 255
Logical Address: 48160, Physical Address: 48160, Value: 255
Logical Address: 4325, Physical Address: 4325, Value: 255
Logical Address: 8695, Physical Address: 8695, Value: 255
Logical Address: 56730, Physical Address: 56730, Value: 255
Logical Address: 50960, Physical Address: 50960, Value: 255
Logical Address: 14995, Physical Address: 14995, Value: 255
Logical Address: 54630, Physical Address: 54630, Value: 255
Logical Address: 51660, Physical Address: 51660, Value: 255
Logical Address: 8960, Physical Address: 8960, Value: 255
Logical Address: 28995, Physical Address: 28995, Value: 255
Logical Address: 38360, Physical Address: 38360, Value: 255
Logical Address: 19195, Physical Address: 19195, Value: 255
Logical Address: 53230, Physical Address: 53230, Value: 255
Logical Address: 63730, Physical Address: 63730, Value: 255
Logical Address: 60230, Physical Address: 60230, Value: 255
Logical Address: 55595, Physical Address: 55595, Value: 255
Logical Address: 35295, Physical Address: 35295, Value: 255
Logical Address: 36260, Physical Address: 36260, Value: 255
Logical Address: 19895, Physical Address: 19895, Value: 255
Logical Address: 47195, Physical Address: 47195, Value: 255
Logical Address: 38095, Physical Address: 38095, Value: 255
Logical Address: 29525, Physical Address: 29525, Value: 255
Logical Address: 295, Physical Address: 295, Value: 255
Logical Address: 59530, Physical Address: 59530, Value: 255
Logical Address: 61630, Physical Address: 61630, Value: 255
Logical Address: 60930, Physical Address: 60930, Value: 255
Logical Address: 49560, Physical Address: 49560, Value: 255
Logical Address: 9660, Physical Address: 9660, Value: 255
Logical Address: 22960, Physical Address: 22960, Value: 255
Logical Address: 12725, Physical Address: 12725, Value: 255
Logical Address: 5895, Physical Address: 5895, Value: 255
Logical Address: 46060, Physical Address: 46060, Value: 255
Logical Address: 5025, Physical Address: 5025, Value: 255
Logical Address: 2660, Physical Address: 2660, Value: 255
Logical Address: 31095, Physical Address: 31095, Value: 255
Logical Address: 37660, Physical Address: 37660, Value: 255
Logical Address: 7825, Physical Address: 7825, Value: 255
Logical Address: 61461, Physical Address: 61461, Value: 255
Logical Address: 11495, Physical Address: 11495, Value: 255
Logical Address: 49995, Physical Address: 49995, Value: 255
Logical Address: 31360, Physical Address: 31360, Value: 255
Logical Address: 9925, Physical Address: 9925, Value: 255
Logical Address: 60761, Physical Address: 60761, Value: 255
Logical Address: 125, Physical Address: 125, Value: 255
Logical Address: 10095, Physical Address: 10095, Value: 255
Logical Address: 44660, Physical Address: 44660, Value: 255
Logical Address: 17095, Physical Address: 17095, Value: 255
Logical Address: 53930, Physical Address: 53930, Value: 255
Logical Address: 57695, Physical Address: 57695, Value: 255
Logical Address: 34595, Physical Address: 34595, Value: 255
Logical Address: 42295, Physical Address: 42295, Value: 255
Logical Address: 28125, Physical Address: 28125, Value: 255
Logical Address: 60496, Physical Address: 60496, Value: 255
Logical Address: 27860, Physical Address: 27860, Value: 255
Logical Address: 22695, Physical Address: 22695, Value: 255
Logical Address: 40460, Physical Address: 40460, Value: 255
Logical Address: 18495, Physical Address: 18495, Value: 255
Logical Address: 41860, Physical Address: 41860, Value: 255
Logical Address: 6425, Physical Address: 6425, Value: 255
Logical Address: 7995, Physical Address: 7995, Value: 255
Logical Address: 45360, Physical Address: 45360, Value: 255
Logical Address: 11060, Physical Address: 11060, Value: 255
Logical Address: 28295, Physical Address: 28295, Value: 255
Logical Address: 44395, Physical Address: 44395, Value: 255
Logical Address: 27425, Physical Address: 27425, Value: 255
Logical Address: 995, Physical Address: 995, Value: 255
Logical Address: 53495, Physical Address: 53495, Value: 255
Logical Address: 35995, Physical Address: 35995, Value: 255
Logical Address: 30225, Physical Address: 30225, Value: 255
Logical Address: 59796, Physical Address: 59796, Value: 255
Logical Address: 33895, Physical Address: 33895, Value: 255
Logical Address: 30925, Physical Address: 30925, Value: 255
Logical Address: 53761, Physical Address: 53761, Value: 255
Logical Address: 8260, Physical Address: 8260, Value: 255
Logical Address: 17625, Physical Address: 17625, Value: 255
Logical Address: 63996, Physical Address: 63996, Value: 255
Logical Address: 32495, Physical Address: 32495, Value: 255
Logical Address: 42995, Physical Address: 42995, Value: 255
Logical Address: 39495, Physical Address: 39495, Value: 255
Logical Address: 34860, Physical Address: 34860, Value: 255
Logical Address: 14560, Physical Address: 14560, Value: 255
Logical Address: 15525, Physical Address: 15525, Value: 255
Logical Address: 64696, Physical Address: 64696, Value: 255
Logical Address: 26460, Physical Address: 26460, Value: 255
Logical Address: 17360, Physical Address: 17360, Value: 255
Logical Address: 26195, Physical Address: 26195, Value: 255
Logical Address: 45095, Physical Address: 45095, Value: 255
Logical Address: 38795, Physical Address: 38795, Value: 255
Logical Address: 40895, Physical Address: 40895, Value: 255
Logical Address: 40195, Physical Address: 40195, Value: 255
Logical Address: 28825, Physical Address: 28825, Value: 255
Logical Address: 54461, Physical Address: 54461, Value: 255
Logical Address: 2225, Physical Address: 2225, Value: 255
Logical Address: 9395, Physical Address: 9395, Value: 255
Logical Address: 50695, Physical Address: 50695, Value: 255
Logical Address: 25325, Physical Address: 25325, Value: 255
Logical Address: 1695, Physical Address: 1695, Value: 255
Logical Address: 47460, Physical Address: 47460, Value: 255
Logical Address: 10360, Physical Address: 10360, Value: 255
Logical Address: 16925, Physical Address: 16925, Value: 255
Logical Address: 4495, Physical Address: 4495, Value: 255
Logical Address: 58130, Physical Address: 58130, Value: 255
Logical Address: 56295, Physical Address: 56295, Value: 255
Logical Address: 29260, Physical Address: 29260, Value: 255
Logical Address: 10625, Physical Address: 10625, Value: 255
Logical Address: 6595, Physical Address: 6595, Value: 255
Logical Address: 57430, Physical Address: 57430, Value: 255
Logical Address: 62330, Physical Address: 62330, Value: 255
Logical Address: 54895, Physical Address: 54895, Value: 255
Logical Address: 23925, Physical Address: 23925, Value: 255
Logical Address: 61896, Physical Address: 61896, Value: 255
Logical Address: 33195, Physical Address: 33195, Value: 255
Logical Address: 36960, Physical Address: 36960, Value: 255
Logical Address: 13860, Physical Address: 13860, Value: 255
Logical Address: 21560, Physical Address: 21560, Value: 255
Logical Address: 24795, Physical Address: 24795, Value: 255
Logical Address: 39760, Physical Address: 39760, Value: 255
Logical Address: 7125, Physical Address: 7125, Value: 255
Logical Address: 1960, Physical Address: 1960, Value: 255
Logical Address: 19725, Physical Address: 19725, Value: 255
Logical Address: 63296, Physical Address: 63296, Value: 255
Logical Address: 21125, Physical Address: 21125, Value: 255
Logical Address: 3095, Physical Address: 3095, Value: 255
Logical Address: 52795, Physical Address: 52795, Value: 255
Logical Address: 24625, Physical Address: 24625, Value: 255
Logical Address: 55861, Physical Address: 55861, Value: 255
Logical Address: 7560, Physical Address: 7560, Value: 255
Logical Address: 23660, Physical Address: 23660, Value: 255
Logical Address: 24095, Physical Address: 24095, Value: 255
Logical Address: 45795, Physical Address: 45795, Value: 255
Logical Address: 32760, Physical Address: 32760, Value: 255
Logical Address: 15260, Physical Address: 15260, Value: 255
Logical Address: 26895, Physical Address: 26895, Value: 255
Logical Address: 39060, Physical Address: 39060, Value: 255
Logical Address: 13160, Physical Address: 13160, Value: 255
Logical Address: 27595, Physical Address: 27595, Value: 255
Logical Address: 33025, Physical Address: 33025, Value: 255
Logical Address: 53061, Physical Address: 53061, Value: 255
Logical Address: 14295, Physical Address: 14295, Value: 255
Logical Address: 43260, Physical Address: 43260, Value: 255
Logical Address: 11760, Physical Address: 11760, Value: 255
Logical Address: 22260, Physical Address: 22260, Value: 255
Logical Address: 18760, Physical Address: 18760, Value: 255
Logical Address: 14125, Physical Address: 14125, Value: 255
Logical Address: 59361, Physical Address: 59361, Value: 255
Logical Address: 12195, Physical Address: 12195, Value: 255
Logical Address: 43960, Physical Address: 43960, Value: 255
Logical Address: 5725, Physical Address: 5725, Value: 255
Logical Address: 62161, Physical Address: 62161, Value: 255
Logical Address: 5460, Physical Address: 5460, Value: 255
Logical Address: 24360, Physical Address: 24360, Value: 255
Logical Address: 18060, Physical Address: 18060, Value: 255
Logical Address: 20160, Physical Address: 20160, Value: 255
Logical Address: 19460, Physical Address: 19460, Value: 255
Logical Address: 25495, Physical Address: 25495, Value: 255
Logical Address: 51130, Physical Address: 51130, Value: 255
Logical Address: 64430, Physical Address: 64430, Value: 255
Logical Address: 54195, Physical Address: 54195, Value: 255
Logical Address: 29960, Physical Address: 29960, Value: 255
Logical Address: 21995, Physical Address: 21995, Value: 255
Logical Address: 46495, Physical Address: 46495, Value: 255
Logical Address: 26725, Physical Address: 26725, Value: 255
Logical Address: 55161, Physical Address: 55161, Value: 255
Logical Address: 13595, Physical Address: 13595, Value: 255
Logical Address: 49295, Physical Address: 49295, Value: 255
Logical Address: 37395, Physical Address: 37395, Value: 255
Logical Address: 35560, Physical Address: 35560, Value: 255
Logical Address: 8525, Physical Address: 8525, Value: 255
Logical Address: 7295, Physical Address: 7295, Value: 255
Logical Address: 51395, Physical Address: 51395, Value: 255
Logical Address: 36695, Physical Address: 36695, Value: 255
Logical Address: 41595, Physical Address: 41595, Value: 255
Logical Address: 34160, Physical Address: 34160, Value: 255
Logical Address: 20595, Physical Address: 20595, Value: 255
Logical Address: 41160, Physical Address: 41160, Value: 255
Logical Address: 12460, Physical Address: 12460, Value: 255
Logical Address: 16225, Physical Address: 16225, Value: 255
Logical Address: 28241, Physical Address: 28241, Value: 255
Logical Address: 55012, Physical Address: 55012, Value: 255
Logical Address: 18441, Physical Address: 18441, Value: 255
Logical Address: 52477, Physical Address: 52477, Value: 255
Logical Address: 62977, Physical Address: 62977, Value: 255
Logical Address: 59477, Physical Address: 59477, Value: 255
Logical Address: 6711, Physical Address: 6711, Value: 255
Logical Address: 51946, Physical Address: 51946, Value: 255
Logical Address: 52911, Physical Address: 52911, Value: 255
Logical Address: 19141, Physical Address: 19141, Value: 255
Logical Address: 63847, Physical Address: 63847, Value: 255
Logical Address: 54746, Physical Address: 54746, Value: 255
Logical Address: 46176, Physical Address: 46176, Value: 255
Logical Address: 65077, Physical Address: 65077, Value: 255
Logical Address: 58777, Physical Address: 58777, Value: 255
Logical Address: 60877, Physical Address: 60877, Value: 255
Logical Address: 60177, Physical Address: 60177, Value: 255
Logical Address: 676, Physical Address: 676, Value: 255
Logical Address: 26311, Physical Address: 26311, Value: 255
Logical Address: 39611, Physical Address: 39611, Value: 255
Logical Address: 29376, Physical Address: 29376, Value: 255
Logical Address: 5141, Physical Address: 5141, Value: 255
Logical Address: 62712, Physical Address: 62712, Value: 255
Logical Address: 21676, Physical Address: 21676, Value: 255
Logical Address: 19311, Physical Address: 19311, Value: 255
Logical Address: 47746, Physical Address: 47746, Value: 255
Logical Address: 54311, Physical Address: 54311, Value: 255
Logical Address: 24476, Physical Address: 24476, Value: 255
Logical Address: 12576, Physical Address: 12576, Value: 255
Logical Address: 10741, Physical Address: 10741, Value: 255
Logical Address: 1111, Physical Address: 1111, Value: 255
Logical Address: 48011, Physical Address: 48011, Value: 255
Logical Address: 26576, Physical Address: 26576, Value: 255
Logical Address: 11876, Physical Address: 11876, Value: 255
Logical Address: 16776, Physical Address: 16776, Value: 255
Logical Address: 9341, Physical Address: 9341, Value: 255
Logical Address: 61312, Physical Address: 61312, Value: 255
Logical Address: 16341, Physical Address: 16341, Value: 255
Logical Address: 53177, Physical Address: 53177, Value: 255
Logical Address: 8811, Physical Address: 8811, Value: 255
Logical Address: 51246, Physical Address: 51246, Value: 255
Logical Address: 58946, Physical Address: 58946, Value: 255
Logical Address: 44776, Physical Address: 44776, Value: 255
Logical Address: 11611, Physical Address: 11611, Value: 255
Logical Address: 44511, Physical Address: 44511, Value: 255
Logical Address: 21941, Physical Address: 21941, Value: 255
Logical Address: 57112, Physical Address: 57112, Value: 255
Logical Address: 17741, Physical Address: 17741, Value: 255
Logical Address: 58512, Physical Address: 58512, Value: 255
Logical Address: 23076, Physical Address: 23076, Value: 255
Logical Address: 7241, Physical Address: 7241, Value: 255
Logical Address: 62012, Physical Address: 62012, Value: 255
Logical Address: 27711, Physical Address: 27711, Value: 255
Logical Address: 27541, Physical Address: 27541, Value: 255
Logical Address: 61047, Physical Address: 61047, Value: 255
Logical Address: 44076, Physical Address: 44076, Value: 255
Logical Address: 241, Physical Address: 241, Value: 255
Logical Address: 4611, Physical Address: 4611, Value: 255
Logical Address: 52646, Physical Address: 52646, Value: 255
Logical Address: 46876, Physical Address: 46876, Value: 255
Logical Address: 10911, Physical Address: 10911, Value: 255
Logical Address: 50546, Physical Address: 50546, Value: 255
Logical Address: 47576, Physical Address: 47576, Value: 255
Logical Address: 4876, Physical Address: 4876, Value: 255
Logical Address: 24911, Physical Address: 24911, Value: 255
Logical Address: 34276, Physical Address: 34276, Value: 255
Logical Address: 15111, Physical Address: 15111, Value: 255
Logical Address: 49146, Physical Address: 49146, Value: 255
Logical Address: 59646, Physical Address: 59646, Value: 255
Logical Address: 56146, Physical Address: 56146, Value: 255
Logical Address: 51511, Physical Address: 51511, Value: 255
Logical Address: 31211, Physical Address: 31211, Value: 255
Logical Address: 32176, Physical Address: 32176, Value: 255
Logical Address: 15811, Physical Address: 15811, Value: 255
Logical Address: 43111, Physical Address: 43111, Value: 255
Logical Address: 34011, Physical Address: 34011, Value: 255
Logical Address: 25441, Physical Address: 25441, Value: 255
Logical Address: 61747, Physical Address: 61747, Value: 255
Logical Address: 55446, Physical Address: 55446, Value: 255
Logical Address: 57546, Physical Address: 57546, Value: 255
Logical Address: 56846, Physical Address: 56846, Value: 255
Logical Address: 45476, Physical Address: 45476, Value: 255
Logical Address: 5576, Physical Address: 5576, Value: 255
Logical Address: 18876, Physical Address: 18876, Value: 255
Logical Address: 8641, Physical Address: 8641, Value: 255
Logical Address: 1811, Physical Address: 1811, Value: 255
Logical Address: 41976, Physical Address: 41976, Value: 255
Logical Address: 941, Physical Address: 941, Value: 255
Logical Address: 64112, Physical Address: 64112, Value: 255
Logical Address: 27011, Physical Address: 27011, Value: 255
Logical Address: 33576, Physical Address: 33576, Value: 255
Logical Address: 3741, Physical Address: 3741, Value: 255
Logical Address: 57377, Physical Address: 57377, Value: 255
Logical Address: 7411, Physical Address: 7411, Value: 255
Logical Address: 45911, Physical Address: 45911, Value: 255
Logical Address: 27276, Physical Address: 27276, Value: 255
Logical Address: 5841, Physical Address: 5841, Value: 255
Logical Address: 56677, Physical Address: 56677, Value: 255
Logical Address: 61577, Physical Address: 61577, Value: 255
Logical Address: 6011, Physical Address: 6011, Value: 255
Logical Address: 40576, Physical Address: 40576, Value: 255
Logical Address: 13011, Physical Address: 13011, Value: 255
Logical Address: 49846, Physical Address: 49846, Value: 255
Logical Address: 53611, Physical Address: 53611, Value: 255
Logical Address: 30511, Physical Address: 30511, Value: 255
Logical Address: 38211, Physical Address: 38211, Value: 255
Logical Address: 24041, Physical Address: 24041, Value: 255
Logical Address: 56412, Physical Address: 56412, Value: 255
Logical Address: 23776, Physical Address: 23776, Value: 255
Logical Address: 18611, Physical Address: 18611, Value: 255
Logical Address: 36376, Physical Address: 36376, Value: 255
Logical Address: 14411, Physical Address: 14411, Value: 255
Logical Address: 37776, Physical Address: 37776, Value: 255
Logical Address: 2341, Physical Address: 2341, Value: 255
Logical Address: 3911, Physical Address: 3911, Value: 255
Logical Address: 41276, Physical Address: 41276, Value: 255
Logical Address: 6976, Physical Address: 6976, Value: 255
Logical Address: 24211, Physical Address: 24211, Value: 255
Logical Address: 40311, Physical Address: 40311, Value: 255
Logical Address: 23341, Physical Address: 23341, Value: 255
Logical Address: 62447, Physical Address: 62447, Value: 255
Logical Address: 49411, Physical Address: 49411, Value: 255
Logical Address: 31911, Physical Address: 31911, Value: 255
Logical Address: 26141, Physical Address: 26141, Value: 255
Logical Address: 55712, Physical Address: 55712, Value: 255
Logical Address: 29811, Physical Address: 29811, Value: 255
Logical Address: 26841, Physical Address: 26841, Value: 255
Logical Address: 49677, Physical Address: 49677, Value: 255
Logical Address: 4176, Physical Address: 4176, Value: 255
Logical Address: 13541, Physical Address: 13541, Value: 255
Logical Address: 59912, Physical Address: 59912, Value: 255
Logical Address: 28411, Physical Address: 28411, Value: 255
Logical Address: 38911, Physical Address: 38911, Value: 255
Logical Address: 35411, Physical Address: 35411, Value: 255
Logical Address: 30776, Physical Address: 30776, Value: 255
Logical Address: 10476, Physical Address: 10476, Value: 255
Logical Address: 11441, Physical Address: 11441, Value: 255
Logical Address: 60612, Physical Address: 60612, Value: 255
Logical Address: 22376, Physical Address: 22376, Value: 255
Logical Address: 13276, Physical Address: 13276, Value: 255
Logical Address: 22111, Physical Address: 22111, Value: 255
Logical Address: 41011, Physical Address: 41011, Value: 255
Logical Address: 34711, Physical Address: 34711, Value: 255
Logical Address: 36811, Physical Address: 36811, Value: 255
Logical Address: 36111, Physical Address: 36111, Value: 255
Logical Address: 24741, Physical Address: 24741, Value: 255
Logical Address: 50377, Physical Address: 50377, Value: 255
Logical Address: 63677, Physical Address: 63677, Value: 255
Logical Address: 5311, Physical Address: 5311, Value: 255
Logical Address: 46611, Physical Address: 46611, Value: 255
Logical Address: 21241, Physical Address: 21241, Value: 255
Logical Address: 63147, Physical Address: 63147, Value: 255
Logical Address: 43376, Physical Address: 43376, Value: 255
Logical Address: 6276, Physical Address: 6276, Value: 255
Logical Address: 12841, Physical Address: 12841, Value: 255
Logical Address: 411, Physical Address: 411, Value: 255
Logical Address: 54046, Physical Address: 54046, Value: 255
Logical Address: 52211, Physical Address: 52211, Value: 255
Logical Address: 25176, Physical Address: 25176, Value: 255
Logical Address: 6541, Physical Address: 6541, Value: 255
Logical Address: 2511, Physical Address: 2511, Value: 255
Logical Address: 53346, Physical Address: 53346, Value: 255
Logical Address: 58246, Physical Address: 58246, Value: 255
Logical Address: 50811, Physical Address: 50811, Value: 255
Logical Address: 19841, Physical Address: 19841, Value: 255
Logical Address: 57812, Physical Address: 57812, Value: 255
Logical Address: 29111, Physical Address: 29111, Value: 255
Logical Address: 32876, Physical Address: 32876, Value: 255
Logical Address: 9776, Physical Address: 9776, Value: 255
Logical Address: 17476, Physical Address: 17476, Value: 255
Logical Address: 20711, Physical Address: 20711, Value: 255
Logical Address: 35676, Physical Address: 35676, Value: 255
Logical Address: 3041, Physical Address: 3041, Value: 255
Logical Address: 63412, Physical Address: 63412, Value: 255
Logical Address: 15641, Physical Address: 15641, Value: 255
Logical Address: 59212, Physical Address: 59212, Value: 255
Logical Address: 17041, Physical Address: 17041, Value: 255
Logical Address: 64547, Physical Address: 64547, Value: 255
Logical Address: 48711, Physical Address: 48711, Value: 255
Logical Address: 20541, Physical Address: 20541, Value: 255
Logical Address: 51777, Physical Address: 51777, Value: 255
Logical Address: 3476, Physical Address: 3476, Value: 255
Logical Address: 19576, Physical Address: 19576, Value: 255
Logical Address: 20011, Physical Address: 20011, Value: 255
Logical Address: 41711, Physical Address: 41711, Value: 255
Logical Address: 28676, Physical Address: 28676, Value: 255
Logical Address: 11176, Physical Address: 11176, Value: 255
Logical Address: 22811, Physical Address: 22811, Value: 255
Logical Address: 34976, Physical Address: 34976, Value: 255
Logical Address: 9076, Physical Address: 9076, Value: 255
Logical Address: 23511, Physical Address: 23511, Value: 255
Logical Address: 28941, Physical Address: 28941, Value: 255
Logical Address: 48977, Physical Address: 48977, Value: 255
Logical Address: 10211, Physical Address: 10211, Value: 255
Logical Address: 39176, Physical Address: 39176, Value: 255
Logical Address: 7676, Physical Address: 7676, Value: 255
Logical Address: 18176, Physical Address: 18176, Value: 255
Logical Address: 14676, Physical Address: 14676, Value: 255
Logical Address: 10041, Physical Address: 10041, Value: 255
Logical Address: 55277, Physical Address: 55277, Value: 255
Logical Address: 8111, Physical Address: 8111, Value: 255
Logical Address: 39876, Physical Address: 39876, Value: 255
Logical Address: 1641, Physical Address: 1641, Value: 255
Logical Address: 58077, Physical Address: 58077, Value: 255
Logical Address: 1376, Physical Address: 1376, Value: 255
Logical Address: 20276, Physical Address: 20276, Value: 255
Logical Address: 13976, Physical Address: 13976, Value: 255
Logical Address: 16076, Physical Address: 16076, Value: 255
Logical Address: 15376, Physical Address: 15376, Value: 255
Logical Address: 21411, Physical Address: 21411, Value: 255
Logical Address: 47046, Physical Address: 47046, Value: 255
Logical Address: 60346, Physical Address: 60346, Value: 255
Logical Address: 50111, Physical Address: 50111, Value: 255
Logical Address: 25876, Physical Address: 25876, Value: 255
Logical Address: 17911, Physical Address: 17911, Value: 255
Logical Address: 42411, Physical Address: 42411, Value: 255
Logical Address: 22641, Physical Address: 22641, Value: 255
Logical Address: 51077, Physical Address: 51077, Value: 255
Logical Address: 9511, Physical Address: 9511, Value: 255
Logical Address: 45211, Physical Address: 45211, Value: 255
Logical Address: 33311, Physical Address: 33311, Value: 255
Logical Address: 31476, Physical Address: 31476, Value: 255
Logical Address: 4441, Physical Address: 4441, Value: 255
Logical Address: 3211, Physical Address: 3211, Value: 255
Logical Address: 47311, Physical Address: 47311, Value: 255
Logical Address: 32611, Physical Address: 32611, Value: 255
Logical Address: 37511, Physical Address: 37511, Value: 255
Logical Address: 30076, Physical Address: 30076, Value: 255
Logical Address: 16511, Physical Address: 16511, Value: 255
Logical Address: 37076, Physical Address: 37076, Value: 255
Logical Address: 8376, Physical Address: 8376, Value: 255
Logical Address: 12141, Physical Address: 12141, Value: 255
Logical Address: 54577, Physical Address: 54577, Value: 255
Logical Address: 62277, Physical Address: 62277, Value: 255
Logical Address: 65512, Physical Address: 65512, Value: 255
Logical Address: 14941, Physical Address: 14941, Value: 255
Logical Address: 65247, Physical Address: 65247, Value: 255
Logical Address: 42676, Physical Address: 42676, Value: 255
Logical Address: 12311, Physical Address: 12311, Value: 255
Logical Address: 38476, Physical Address: 38476, Value: 255
Logical Address: 13711, Physical Address: 13711, Value: 255
Logical Address: 43811, Physical Address: 43811, Value: 255
Logical Address: 27976, Physical Address: 27976, Value: 255
Logical Address: 17211, Physical Address: 17211, Value: 255
Logical Address: 48446, Physical Address: 48446, Value: 255
Logical Address: 48276, Physical Address: 48276, Value: 255
Logical Address: 64377, Physical Address: 64377, Value: 255
Logical Address: 64812, Physical Address: 64812, Value: 255
Logical Address: 20976, Physical Address: 20976, Value: 255
Logical Address: 7941, Physical Address: 7941, Value: 255
Logical Address: 55977, Physical Address: 55977, Value: 255
Logical Address: 2076, Physical Address: 2076, Value: 255
Logical Address: 14241, Physical Address: 14241, Value: 255
Logical Address: 53877, Physical Address: 53877, Value: 255
Logical Address: 2776, Physical Address: 2776, Value: 255
Logical Address: 25611, Physical Address: 25611, Value: 255
Logical Address: 28241, Physical Address: 28241, Value: 255
Logical Address: 55012, Physical Address: 55012, Value: 255
Logical Address: 18441, Physical Address: 18441, Value: 255
Logical Address: 52477, Physical Address: 52477, Value: 255
Logical Address: 62977, Physical Address: 62977, Value: 255
Logical Address: 59477, Physical Address: 59477, Value: 255
Logical Address: 6711, Physical Address: 6711, Value: 255
Logical Address: 51946, Physical Address: 51946, Value: 255
Logical Address: 52911, Physical Address: 52911, Value: 255
Logical Address: 19141, Physical Address: 19141, Value: 255
Logical Address: 63847, Physical Address: 63847, Value: 255
Logical Address: 54746, Physical Address: 54746, Value: 255
Logical Address: 46176, Physical Address: 46176, Value: 255
Logical Address: 65077, Physical Address: 65077, Value: 255
Logical Address: 58777, Physical Address: 58777, Value: 255
Logical Address: 60877, Physical Address: 60877, Value: 255
Logical Address: 60177, Physical Address: 60177, Value: 255
Logical Address: 676, Physical Address: 676, Value: 255
Logical Address: 26311, Physical Address: 26311, Value: 255
Logical Address: 39611, Physical Address: 39611, Value: 255
Logical Address: 29376, Physical Address: 29376, Value: 255
Logical Address: 5141, Physical Address: 5141, Value: 255
Logical Address: 62712, Physical Address: 62712, Value: 255
Logical Address: 21676, Physical Address: 21676, Value: 255
Logical Address: 19311, Physical Address: 19311, Value: 255
Logical Address: 47746, Physical Address: 47746, Value: 255
Logical Address: 54311, Physical Address: 54311, Value: 255
Logical Address: 24476, Physical Address: 24476, Value: 255
Logical Address: 12576, Physical Address: 12576, Value: 255
Logical Address: 10741, Physical Address: 10741, Value: 255
Logical Address: 1111, Physical Address: 1111, Value: 255
Logical Address: 48011, Physical Address: 48011, Value: 255
Logical Address: 26576, Physical Address: 26576, Value: 255
Logical Address: 11876, Physical Address: 11876, Value: 255
Logical Address: 16776, Physical Address: 16776, Value: 255
Logical Address: 9341, Physical Address: 9341, Value: 255
Logical Address: 61312, Physical Address: 61312, Value: 255
Logical Address: 16341, Physical Address: 16341, Value: 255
Logical Address: 53177, Physical Address: 53177, Value: 255
Logical Address: 8811, Physical Address: 8811, Value: 255
Logical Address: 51246, Physical Address: 51246, Value: 255
Logical Address: 58946, Physical Address: 58946, Value: 255
Logical Address: 44776, Physical Address: 44776, Value: 255
Logical Address: 11611, Physical Address: 11611, Value: 255
Logical Address: 44511, Physical Address: 44511, Value: 255
Logical Address: 21941, Physical Address: 21941, Value: 255
Logical Address: 57112, Physical Address: 57112, Value: 255
Logical Address: 17741, Physical Address: 17741, Value: 255
Logical Address: 58512, Physical Address: 58512, Value: 255
Logical Address: 23076, Physical Address: 23076, Value: 255
Logical Address: 7241, Physical Address: 7241, Value: 255
Logical Address: 62012, Physical Address: 62012, Value: 255
Logical Address: 27711, Physical Address: 27711, Value: 255
Logical Address: 27541, Physical Address: 27541, Value: 255
Logical Address: 61047, Physical Address: 61047, Value: 255
Logical Address: 44076, Physical Address: 44076, Value: 255
Logical Address: 241, Physical Address: 241, Value: 255
Logical Address: 4611, Physical Address: 4611, Value: 255
Logical Address: 52646, Physical Address: 52646, Value: 255
Logical Address: 46876, Physical Address: 46876, Value: 255
Logical Address: 10911, Physical Address: 10911, Value: 255
Logical Address: 50546, Physical Address: 50546, Value: 255
Logical Address: 47576, Physical Address: 47576, Value: 255
Logical Address: 4876, Physical Address: 4876, Value: 255
Logical Address: 24911, Physical Address: 24911, Value: 255
Logical Address: 34276, Physical Address: 34276, Value: 255
Logical Address: 15111, Physical Address: 15111, Value: 255
Logical Address: 49146, Physical Address: 49146, Value: 255
Logical Address: 59646, Physical Address: 59646, Value: 255
Logical Address: 56146, Physical Address: 56146, Value: 255
Logical Address: 51511, Physical Address: 51511, Value: 255
Logical Address: 31211, Physical Address: 31211, Value: 255
Logical Address: 32176, Physical Address: 32176, Value: 255
Logical Address: 15811, Physical Address: 15811, Value: 255
Logical Address: 43111, Physical Address: 43111, Value: 255
Logical Address: 34011, Physical Address: 34011, Value: 255
Logical Address: 25441, Physical Address: 25441, Value: 255
Logical Address: 61747, Physical Address: 61747, Value: 255
Logical Address: 55446, Physical Address: 55446, Value: 255
Logical Address: 57546, Physical Address: 57546, Value: 255
Logical Address: 56846, Physical Address: 56846, Value: 255
Logical Address: 45476, Physical Address: 45476, Value: 255
Logical Address: 5576, Physical Address: 5576, Value: 255
Logical Address: 18876, Physical Address: 18876, Value: 255
Logical Address: 8641, Physical Address: 8641, Value: 255
Logical Address: 1811, Physical Address: 1811, Value: 255
Logical Address: 41976, Physical Address: 41976, Value: 255
Logical Address: 941, Physical Address: 941, Value: 255
Logical Address: 64112, Physical Address: 64112, Value: 255
Logical Address: 27011, Physical Address: 27011, Value: 255
Logical Address: 33576, Physical Address: 33576, Value: 255
Logical Address: 3741, Physical Address: 3741, Value: 255
Logical Address: 57377, Physical Address: 57377, Value: 255
Logical Address: 7411, Physical Address: 7411, Value: 255
Logical Address: 45911, Physical Address: 45911, Value: 255
Logical Address: 27276, Physical Address: 27276, Value: 255
Logical Address: 5841, Physical Address: 5841, Value: 255
Logical Address: 56677, Physical Address: 56677, Value: 255
Logical Address: 61577, Physical Address: 61577, Value: 255
Logical Address: 6011, Physical Address: 6011, Value: 255
Logical Address: 40576, Physical Address: 40576, Value: 255
Logical Address: 13011, Physical Address: 13011, Value: 255
Logical Address: 49846, Physical Address: 49846, Value: 255
Logical Address: 53611, Physical Address: 53611, Value: 255
Logical Address: 30511, Physical Address: 30511, Value: 255
Logical Address: 38211, Physical Address: 38211, Value: 255
Logical Address: 24041, Physical Address: 24041, Value: 255
Logical Address: 56412, Physical Address: 56412, Value: 255
Logical Address: 23776, Physical Address: 23776, Value: 255
Logical Address: 18611, Physical Address: 18611, Value: 255
Logical Address: 36376, Physical Address: 36376, Value: 255
Logical Address: 14411, Physical Address: 14411, Value: 255
Logical Address: 37776, Physical Address: 37776, Value: 255
Logical Address: 2341, Physical Address: 2341, Value: 255
Logical Address: 3911, Physical Address: 3911, Value: 255
Logical Address: 41276, Physical Address: 41276, Value: 255
Logical Address: 6976, Physical Address: 6976, Value: 255
Logical Address: 24211, Physical Address: 24211, Value: 255
Logical Address: 40311, Physical Address: 40311, Value: 255
Logical Address: 23341, Physical Address: 23341, Value: 255
Logical Address: 62447, Physical Address: 62447, Value: 255
Logical Address: 49411, Physical Address: 49411, Value: 255
Logical Address: 31911, Physical Address: 31911, Value: 255
Logical Address: 26141, Physical Address: 26141, Value: 255
Logical Address: 55712, Physical Address: 55712, Value: 255
Logical Address: 29811, Physical Address: 29811, Value: 255
Logical Address: 26841, Physical Address: 26841, Value: 255
Logical Address: 49677, Physical Address: 49677, Value: 255
Logical Address: 4176, Physical Address: 4176, Value: 255
Logical Address: 13541, Physical Address: 13541, Value: 255
Logical Address: 59912, Physical Address: 59912, Value: 255
Logical Address: 28411, Physical Address: 28411, Value: 255
Logical Address: 38911, Physical Address: 38911, Value: 255
Logical Address: 35411, Physical Address: 35411, Value: 255
Logical Address: 30776, Physical Address: 30776, Value: 255
Logical Address: 10476, Physical Address: 10476, Value: 255
Logical Address: 11441, Physical Address: 11441, Value: 255
Logical Address: 60612, Physical Address: 60612, Value: 255
Logical Address: 22376, Physical Address: 22376, Value: 255
Logical Address: 13276, Physical Address: 13276, Value: 255
Logical Address: 22111, Physical Address: 22111, Value: 255
Logical Address: 41011, Physical Address: 41011, Value: 255
Logical Address: 34711, Physical Address: 34711, Value: 255
Logical Address: 36811, Physical Address: 36811, Value: 255
Logical Address: 36111, Physical Address: 36111, Value: 255
Logical Address: 24741, Physical Address: 24741, Value: 255
Logical Address: 50377, Physical Address: 50377, Value: 255
Logical Address: 63677, Physical Address: 63677, Value: 255
Logical Address: 5311, Physical Address: 5311, Value: 255
Logical Address: 46611, Physical Address: 46611, Value: 255
Logical Address: 21241, Physical Address: 21241, Value: 255
Logical Address: 63147, Physical Address: 63147, Value: 255
Logical Address: 43376, Physical Address: 43376, Value: 255
Logical Address: 6276, Physical Address: 6276, Value: 255
Logical Address: 12841, Physical Address: 12841, Value: 255
Logical Address: 411, Physical Address: 411, Value: 255
Logical Address: 54046, Physical Address: 54046, Value: 255
Logical Address: 52211, Physical Address: 52211, Value: 255
Logical Address: 25176, Physical Address: 25176, Value: 255
Logical Address: 6541, Physical Address: 6541, Value: 255
Logical Address: 2511, Physical Address: 2511, Value: 255
Logical Address: 53346, Physical Address: 53346, Value: 255
Logical Address: 58246, Physical Address: 58246, Value: 255
Logical Address: 50811, Physical Address: 50811, Value: 255
Logical Address: 19841, Physical Address: 19841, Value: 255
Logical Address: 57812, Physical Address: 57812, Value: 255
Logical Address: 29111, Physical Address: 29111, Value: 255
Logical Address: 32876, Physical Address: 32876, Value: 255
Logical Address: 9776, Physical Address: 9776, Value: 255
Logical Address: 17476, Physical Address: 17476, Value: 255
Logical Address: 20711, Physical Address: 20711, Value: 255
Logical Address: 35676, Physical Address: 35676, Value: 255
Logical Address: 3041, Physical Address: 3041, Value: 255
Logical Address: 63412, Physical Address: 63412, Value: 255
Logical Address: 15641, Physical Address: 15641, Value: 255
Logical Address: 59212, Physical Address: 59212, Value: 255
Logical Address: 17041, Physical Address: 17041, Value: 255
Logical Address: 64547, Physical Address: 64547, Value: 255
Logical Address: 48711, Physical Address: 48711, Value: 255
Logical Address: 20541, Physical Address: 20541, Value: 255
Logical Address: 51777, Physical Address: 51777, Value: 255
Logical Address: 3476, Physical Address: 3476, Value: 255
Logical Address: 19576, Physical Address: 19576, Value: 255
Logical Address: 20011, Physical Address: 20011, Value: 255
Logical Address: 41711, Physical Address: 41711, Value: 255
Logical Address: 28676, Physical Address: 28676, Value: 255
Logical Address: 11176, Physical Address: 11176, Value: 255
Logical Address: 22811, Physical Address: 22811, Value: 255
Logical Address: 34976, Physical Address: 34976, Value: 255
Logical Address: 9076, Physical Address: 9076, Value: 255
Logical Address: 23511, Physical Address: 23511, Value: 255
Logical Address: 28941, Physical Address: 28941, Value: 255
Logical Address: 48977, Physical Address: 48977, Value: 255
Logical Address: 10211, Physical Address: 10211, Value: 255
Logical Address: 39176, Physical Address: 39176, Value: 255
Logical Address: 7676, Physical Address: 7676, Value: 255
Logical Address: 18176, Physical Address: 18176, Value: 255
Logical Address: 14676, Physical Address: 14676, Value: 255
Logical Address: 10041, Physical Address: 10041, Value: 255
Logical Address: 55277, Physical Address: 55277, Value: 255
Logical Address: 8111, Physical Address: 8111, Value: 255
Logical Address: 39876, Physical Address: 39876, Value: 255
Logical Address: 1641, Physical Address: 1641, Value: 255
Logical Address: 58077, Physical Address: 58077, Value: 255
Logical Address: 1376, Physical Address: 1376, Value: 255
Logical Address: 20276, Physical Address: 20276, Value: 255
Logical Address: 13976, Physical Address: 13976, Value: 255
Logical Address: 16076, Physical Address: 16076, Value: 255
Logical Address: 15376, Physical Address: 15376, Value: 255
Logical Address: 21411, Physical Address: 21411, Value: 255
Logical Address: 47046, Physical Address: 47046, Value: 255
Logical Address: 60346, Physical Address: 60346, Value: 255
Logical Address: 50111, Physical Address: 50111, Value: 255
Logical Address: 25876, Physical Address: 25876, Value: 255
Logical Address: 17911, Physical Address: 17911, Value: 255
Logical Address: 42411, Physical Address: 42411, Value: 255
Logical Address: 22641, Physical Address: 22641, Value: 255
Logical Address: 51077, Physical Address: 51077, Value: 255
Logical Address: 9511, Physical Address: 9511, Value: 255
Logical Address: 45211, Physical Address: 45211, Value: 255
Logical Address: 33311, Physical Address: 33311, Value: 255
Logical Address: 31476, Physical Address: 31476, Value: 255
Logical Address: 4441, Physical Address: 4441, Value: 255
Logical Address: 3211, Physical Address: 3211, Value: 255
Logical Address: 47311, Physical Address: 47311, Value: 255
Logical Address: 32611, Physical Address: 32611, Value: 255
Logical Address: 37511, Physical Address: 37511, Value: 255
Logical Address: 30076, Physical Address: 30076, Value: 255
Logical Address: 16511, Physical Address: 16511, Value: 255
Logical Address: 41160, Physical Address: 41160, Value: 255
Logical Address: 12460, Physical Address: 12460, Value: 255
Logical Address: 16225, Physical Address: 16225, Value: 255
Logical Address: 58661, Physical Address: 58661, Value: 255
Logical Address: 825, Physical Address: 825, Value: 255
Logical Address: 4060, Physical Address: 4060, Value: 255
Logical Address: 19025, Physical Address: 19025, Value: 255
Logical Address: 3795, Physical Address: 3795, Value: 255
Logical Address: 46760, Physical Address: 46760, Value: 255
Logical Address: 16395, Physical Address: 16395, Value: 255
Logical Address: 42560, Physical Address: 42560, Value: 255
Logical Address: 17795, Physical Address: 17795, Value: 255
Logical Address: 47895, Physical Address: 47895, Value: 255
Logical Address: 32060, Physical Address: 32060, Value: 255
Logical Address: 21295, Physical Address: 21295, Value: 255
Logical Address: 52530, Physical Address: 52530, Value: 255
Logical Address: 52360, Physical Address: 52360, Value: 255
Logical Address: 2925, Physical Address: 2925, Value: 255
Logical Address: 3360, Physical Address: 3360, Value: 255
Logical Address: 25060, Physical Address: 25060, Value: 255
Logical Address: 12025, Physical Address: 12025, Value: 255
Logical Address: 60061, Physical Address: 60061, Value: 255
Logical Address: 6160, Physical Address: 6160, Value: 255
Logical Address: 18325, Physical Address: 18325, Value: 255
Logical Address: 57961, Physical Address: 57961, Value: 255
Logical Address: 6860, Physical Address: 6860, Value: 255
Logical Address: 29695, Physical Address: 29695, Value: 255
Logical Address: 32325, Physical Address: 32325, Value: 255
Logical Address: 59096, Physical Address: 59096, Value: 255
Logical Address: 22525, Physical Address: 22525, Value: 255
Logical Address: 56561, Physical Address: 56561, Value: 255
Logical Address: 1525, Physical Address: 1525, Value: 255
Logical Address: 63561, Physical Address: 63561, Value: 255
Logical Address: 10795, Physical Address: 10795, Value: 255
Logical Address: 56030, Physical Address: 56030, Value: 255
Logical Address: 56995, Physical Address: 56995, Value: 255
Logical Address: 23225, Physical Address: 23225, Value: 255
Logical Address: 2395, Physical Address: 2395, Value: 255
Logical Address: 58830, Physical Address: 58830, Value: 255
Logical Address: 50260, Physical Address: 50260, Value: 255
Logical Address: 3625, Physical Address: 3625, Value: 255
Logical Address: 62861, Physical Address: 62861, Value: 255
Logical Address: 64961, Physical Address: 64961, Value: 255
Logical Address: 64261, Physical Address: 64261, Value: 255
Logical Address: 4760, Physical Address: 4760, Value: 255
Logical Address: 30395, Physical Address: 30395, Value: 255
Logical Address: 43695, Physical Address: 43695, Value: 255
Logical Address: 33460, Physical Address: 33460, Value: 255
Logical Address: 9225, Physical Address: 9225, Value: 255
Logical Address: 1260, Physical Address: 1260, Value: 255
Logical Address: 25760, Physical Address: 25760, Value: 255
Logical Address: 23395, Physical Address: 23395, Value: 255
Logical Address: 51830, Physical Address: 51830, Value: 255
Logical Address: 58395, Physical Address: 58395, Value: 255
Logical Address: 28560, Physical Address: 28560, Value: 255
Logical Address: 16660, Physical Address: 16660, Value: 255
Logical Address: 14825, Physical Address: 14825, Value: 255
Logical Address: 5195, Physical Address: 5195, Value: 255
Logical Address: 52095, Physical Address: 52095, Value: 255
Logical Address: 30660, Physical Address: 30660, Value: 255
Logical Address: 15960, Physical Address: 15960, Value: 255
Logical Address: 20860, Physical Address: 20860, Value: 255
Logical Address: 13425, Physical Address: 13425, Value: 255
Logical Address: 65396, Physical Address: 65396, Value: 255
Logical Address: 20425, Physical Address: 20425, Value: 255
Logical Address: 57261, Physical Address: 57261, Value: 255
Logical Address: 12895, Physical Address: 12895, Value: 255
Logical Address: 55330, Physical Address: 55330, Value: 255
Logical Address: 63030, Physical Address: 63030, Value: 255
Logical Address: 48860, Physical Address: 48860, Value: 255
Logical Address: 15695, Physical Address: 15695, Value: 255
Logical Address: 48595, Physical Address: 48595, Value: 255
Logical Address: 26025, Physical Address: 26025, Value: 255
Logical Address: 61196, Physical Address: 61196, Value: 255
Logical Address: 21825, Physical Address: 21825, Value: 255
Logical Address: 62596, Physical Address: 62596, Value: 255
Logical Address: 27160, Physical Address: 27160, Value: 255
Logical Address: 11325, Physical Address: 11325, Value: 255
Logical Address: 560, Physical Address: 560, Value: 255
Logical Address: 31795, Physical Address: 31795, Value: 255
Logical Address: 31625, Physical Address: 31625, Value: 255
Logical Address: 65131, Physical Address: 65131, Value: 255
Logical Address: 48160, Physical Address: 48160, Value: 255
Logical Address: 4325, Physical Address: 4325, Value: 255
Logical Address: 8695, Physical Address: 8695, Value: 255
Logical Address: 56730, Physical Address: 56730, Value: 255
Logical Address: 50960, Physical Address: 50960, Value: 255
Logical Address: 14995, Physical Address: 14995, Value: 255
Logical Address: 54630, Physical Address: 54630, Value: 255
Logical Address: 51660, Physical Address: 51660, Value: 255
Logical Address: 8960, Physical Address: 8960, Value: 255
Logical Address: 28995, Physical Address: 28995, Value: 255
Logical Address: 38360, Physical Address: 38360, Value: 255
Logical Address: 19195, Physical Address: 19195, Value: 255
Logical Address: 53230, Physical Address: 53230, Value: 255
Logical Address: 63730, Physical Address: 63730, Value: 255
Logical Address: 60230, Physical Address: 60230, Value: 255
Logical Address: 55595, Physical Address: 55595, Value: 255
Logical Address: 35295, Physical Address: 35295, Value: 255
Logical Address: 36260, Physical Address: 36260, Value: 255
Logical Address: 19895, Physical Address: 19895, Value: 255
Logical Address: 47195, Physical Address: 47195, Value: 255
Logical Address: 38095, Physical Address: 38095, Value: 255
Logical Address: 29525, Physical Address: 29525, Value: 255
Logical Address: 295, Physical Address: 295, Value: 255
Logical Address: 59530, Physical Address: 59530, Value: 255
Logical Address: 61630, Physical Address: 61630, Value: 255
Logical Address: 60930, Physical Address: 60930, Value: 255
Logical Address: 49560, Physical Address: 49560, Value: 255
Logical Address: 9660, Physical Address: 9660, Value: 255
Logical Address: 22960, Physical Address: 22960, Value: 255
Logical Address: 12725, Physical Address: 12725, Value: 255
Logical Address: 5895, Physical Address: 5895, Value: 255
Logical Address: 46060, Physical Address: 46060, Value: 255
Logical Address: 5025, Physical Address: 5025, Value: 255
Logical Address: 2660, Physical Address: 2660, Value: 255
Logical Address: 31095, Physical Address: 31095, Value: 255
Logical Address: 37660, Physical Address: 37660, Value: 255
Logical Address: 7825, Physical Address: 7825, Value: 255
Logical Address: 61461, Physical Address: 61461, Value: 255
Logical Address: 11495, Physical Address: 11495, Value: 255
Logical Address: 49995, Physical Address: 49995, Value: 255
Logical Address: 31360, Physical Address: 31360, Value: 255
Logical Address: 9925, Physical Address: 9925, Value: 255
Logical Address: 60761, Physical Address: 60761, Value: 255
Logical Address: 125, Physical Address: 125, Value: 255
Logical Address: 10095, Physical Address: 10095, Value: 255
Logical Address: 44660, Physical Address: 44660, Value: 255
Logical Address: 17095, Physical Address: 17095, Value: 255
Logical Address: 53930, Physical Address: 53930, Value: 255
Logical Address: 57695, Physical Address: 57695, Value: 255
Logical Address: 34595, Physical Address: 34595, Value: 255
Logical Address: 42295, Physical Address: 42295, Value: 255
Logical Address: 28125, Physical Address: 28125, Value: 255
Logical Address: 60496, Physical Address: 60496, Value: 255
Logical Address: 27860, Physical Address: 27860, Value: 255
Logical Address: 22695, Physical Address: 22695, Value: 255
Logical Address: 40460, Physical Address: 40460, Value: 255
Logical Address: 18495, Physical Address: 18495, Value: 255
Logical Address: 41860, Physical Address: 41860, Value: 255
Logical Address: 6425, Physical Address: 6425, Value: 255
Logical Address: 7995, Physical Address: 7995, Value: 255
Logical Address: 45360, Physical Address: 45360, Value: 255
Logical Address: 11060, Physical Address: 11060, Value: 255
Logical Address: 28295, Physical Address: 28295, Value: 255
Logical Address: 44395, Physical Address: 44395, Value: 255
Logical Address: 27425, Physical Address: 27425, Value: 255
Logical Address: 995, Physical Address: 995, Value: 255
Logical Address: 53495, Physical Address: 53495, Value: 255
Logical Address: 35995, Physical Address: 35995, Value: 255
Logical Address: 30225, Physical Address: 30225, Value: 255
Logical Address: 59796, Physical Address: 59796, Value: 255
Logical Address: 33895, Physical Address: 33895, Value: 255
Logical Address: 30925, Physical Address: 30925, Value: 255
Logical Address: 53761, Physical Address: 53761, Value: 255
Logical Address: 8260, Physical Address: 8260, Value: 255
Logical Address: 17625, Physical Address: 17625, Value: 255
Logical Address: 63996, Physical Address: 63996, Value: 255
Logical Address: 32495, Physical Address: 32495, Value: 255
Logical Address: 42995, Physical Address: 42995, Value: 255
Logical Address: 39495, Physical Address: 39495, Value: 255
Logical Address: 34860, Physical Address: 34860, Value: 255
Logical Address: 14560, Physical Address: 14560, Value: 255
Logical Address: 15525, Physical Address: 15525, Value: 255
Logical Address: 64696, Physical Address: 64696, Value: 255
Logical Address: 26460, Physical Address: 26460, Value: 255
Logical Address: 17360, Physical Address: 17360, Value: 255
Logical Address: 26195, Physical Address: 26195, Value: 255
Logical Address: 45095, Physical Address: 45095, Value: 255
Logical Address: 38795, Physical Address: 38795, Value: 255
Logical Address: 40895, Physical Address: 40895, Value: 255
Logical Address: 40195, Physical Address: 40195, Value: 255
Logical Address: 28825, Physical Address: 28825, Value: 255
Logical Address: 54461, Physical Address: 54461, Value: 255
Logical Address: 2225, Physical Address: 2225, Value: 255
Logical Address: 9395, Physical Address: 9395, Value: 255
Logical Address: 50695, Physical Address: 50695, Value: 255
Logical Address: 25325, Physical Address: 25325, Value: 255
Logical Address: 1695, Physical Address: 1695, Value: 255
Logical Address: 47460, Physical Address: 47460, Value: 255
Logical Address: 10360, Physical Address: 10360, Value: 255
Logical Address: 16925, Physical Address: 16925, Value: 255
Logical Address: 4495, Physical Address: 4495, Value: 255
Logical Address: 58130, Physical Address: 58130, Value: 255
Logical Address: 56295, Physical Address: 56295, Value: 255
Logical Address: 29260, Physical Address: 29260, Value: 255
Logical Address: 10625, Physical Address: 10625, Value: 255
Logical Address: 6595, Physical Address: 6595, Value: 255
Logical Address: 57430, Physical Address: 57430, Value: 255
Logical Address: 62330, Physical Address: 62330, Value: 255
Logical Address: 54895, Physical Address: 54895, Value: 255
Logical Address: 23925, Physical Address: 23925, Value: 255
Logical Address: 61896, Physical Address: 61896, Value: 255
Logical Address: 33195, Physical Address: 33195, Value: 255
Logical Address: 36960, Physical Address: 36960, Value: 255
Logical Address: 13860, Physical Address: 13860, Value: 255
Logical Address: 21560, Physical Address: 21560, Value: 255
Logical Address: 24795, Physical Address: 24795, Value: 255
Logical Address: 39760, Physical Address: 39760, Value: 255
Logical Address: 7125, Physical Address: 7125, Value: 255
Logical Address: 1960, Physical Address: 1960, Value: 255
Logical Address: 19725, Physical Address: 19725, Value: 255
Logical Address: 63296, Physical Address: 63296, Value: 255
Logical Address: 21125, Physical Address: 21125, Value: 255
Logical Address: 3095, Physical Address: 3095, Value: 255
Logical Address: 52795, Physical Address: 52795, Value: 255
Logical Address: 24625, Physical Address: 24625, Value: 255
Logical Address: 55861, Physical Address: 55861, Value: 255
Logical Address: 7560, Physical Address: 7560, Value: 255
Logical Address: 23660, Physical Address: 23660, Value: 255
Logical Address: 24095, Physical Address: 24095, Value: 255
Logical Address: 45795, Physical Address: 45795, Value: 255
Logical Address: 32760, Physical Address: 32760, Value: 255
Logical Address: 15260, Physical Address: 15260, Value: 255
Logical Address: 26895, Physical Address: 26895, Value: 255
Logical Address: 39060, Physical Address: 39060, Value: 255
Logical Address: 13160, Physical Address: 13160, Value: 255
Logical Address: 27595, Physical Address: 27595, Value: 255
Logical Address: 33025, Physical Address: 33025, Value: 255
Logical Address: 53061, Physical Address: 53061, Value: 255
Logical Address: 14295, Physical Address: 14295, Value: 255
Logical Address: 43260, Physical Address: 43260, Value: 255
Logical Address: 11760, Physical Address: 11760, Value: 255
Logical Address: 22260, Physical Address: 22260, Value: 255
Logical Address: 18760, Physical Address: 18760, Value: 255
Logical Address: 14125, Physical Address: 14125, Value: 255
Logical Address: 59361, Physical Address: 59361, Value: 255
Logical Address: 12195, Physical Address: 12195, Value: 255
Logical Address: 43960, Physical Address: 43960, Value: 255
Logical Address: 5725, Physical Address: 5725, Value: 255
Logical Address: 62161, Physical Address: 62161, Value: 255
Logical Address: 5460, Physical Address: 5460, Value: 255
Logical Address: 24360, Physical Address: 24360, Value: 255
Logical Address: 18060, Physical Address: 18060, Value: 255
Logical Address: 20160, Physical Address: 20160, Value: 255
Logical Address: 19460, Physical Address: 19460, Value: 255
Logical Address: 25495, Physical Address: 25495, Value: 255
Logical Address: 51130, Physical Address: 51130, Value: 255
Logical Address: 64430, Physical Address: 64430, Value: 255
Logical Address: 54195, Physical Address: 54195, Value: 255
Logical Address: 29960, Physical Address: 29960, Value: 255
Logical Address: 21995, Physical Address: 21995, Value: 255
Logical Address: 46495, Physical Address: 46495, Value: 255
Logical Address: 26725, Physical Address: 26725, Value: 255
Logical Address: 55161, Physical Address: 55161, Value: 255
Logical Address: 13595, Physical Address: 13595, Value: 255
Logical Address: 49295, Physical Address: 49295, Value: 255
Logical Address: 37395, Physical Address: 37395, Value: 255
Logical Address: 35560, Physical Address: 35560, Value: 255
Logical Address: 8525, Physical Address: 8525, Value: 255
Logical Address: 7295, Physical Address: 7295, Value: 255
Logical Address: 51395, Physical Address: 51395, Value: 255
Logical Address: 36695, Physical Address: 36695, Value: 255
Logical Address: 41595, Physical Address: 41595, Value: 255
Logical Address: 34160, Physical Address: 34160, Value: 255
Logical Address: 20595, Physical Address: 20595, Value: 255
Logical Address: 41160, Physical Address: 41160, Value: 255
Logical Address: 12460, Physical Address: 12460, Value: 255
Logical Address: 16225, Physical Address: 16225, Value: 255
Logical Address: 58661, Physical Address: 58661, Value: 255
Logical Address: 825, Physical Address: 825, Value: 255
Logical Address: 4060, Physical Address: 4060, Value: 255
Logical Address: 19025, Physical Address: 19025, Value: 255
Logical Address: 3795, Physical Address: 3795, Value: 255
Logical Address: 46760, Physical Address: 46760, Value: 255
Logical Address: 16395, Physical Address: 16395, Value: 255
Logical Address: 42560, Physical Address: 42560, Value: 255
Logical Address: 17795, Physical Address: 17795, Value: 255
Logical Address: 47895, Physical Address: 47895, Value: 255
Logical Address: 32060, Physical Address: 32060, Value: 255
Logical Address: 21295, Physical Address: 21295, Value: 255
Logical Address: 52530, Physical Address: 52530, Value: 255
Logical Address: 52360, Physical Address: 52360, Value: 255
Logical Address: 2925, Physical Address: 2925, Value: 255
Logical Address: 3360, Physical Address: 3360, Value: 255
Logical Address: 25060, Physical Address: 25060, Value: 255
Logical Address: 12025, Physical Address: 12025, Value: 255
Logical Address: 60061, Physical Address: 60061, Value: 255
Logical Address: 6160, Physical Address: 6160, Value: 255
Logical Address: 18325, Physical Address: 18325, Value: 255
Logical Address: 57961, Physical Address: 57961, Value: 255
Logical Address: 6860, Physical Address: 6860, Value: 255
Logical Address: 29695, Physical Address: 29695, Value: 255
Logical Address: 32325, Physical Address: 32325, Value: 255
Logical Address: 59096, Physical Address: 59096, Value: 255
Logical Address: 22525, Physical Address: 22525, Value: 255
Logical Address: 56561, Physical Address: 56561, Value: 255
Logical Address: 1525, Physical Address: 1525, Value: 255
Logical Address: 63561, Physical Address: 63561, Value: 255
Logical Address: 10795, Physical Address: 10795, Value: 255
Logical Address: 56030, Physical Address: 56030, Value: 255
Logical Address: 56995, Physical Address: 56995, Value: 255
Logical Address: 23225, Physical Address: 23225, Value: 255
Logical Address: 2395, Physical Address: 2395, Value: 255
Logical Address: 58830, Physical Address: 58830, Value: 255
Logical Address: 50260, Physical Address: 50260, Value: 255
Logical Address: 3625, Physical Address: 3625, Value: 255
Logical Address: 62861, Physical Address: 62861, Value: 255
Logical Address: 64961, Physical Address: 64961, Value: 255
Logical Address: 64261, Physical Address: 64261, Value: 255
Logical Address: 4760, Physical Address: 4760, Value: 255
Logical Address: 30395, Physical Address: 30395, Value: 255
Logical Address: 43695, Physical Address: 43695, Value: 255
Logical Address: 33460, Physical Address: 33460, Value: 255
Logical Address: 9225, Physical Address: 9225, Value: 255
Logical Address: 1260, Physical Address: 1260, Value: 255
Logical Address: 25760, Physical Address: 25760, Value: 255
Logical Address: 23395, Physical Address: 23395, Value: 255
Logical Address: 51830, Physical Address: 51830, Value: 255
Logical Address: 58395, Physical Address: 58395, Value: 255
Logical Address: 28560, Physical Address: 28560, Value: 255
Logical Address: 16660, Physical Address: 16660, Value: 255
Logical Address: 14825, Physical Address: 14825, Value: 255
Logical Address: 5195, Physical Address: 5195, Value: 255
Logical Address: 52095, Physical Address: 52095, Value: 255
Logical Address: 30660, Physical Address: 30660, Value: 255
Logical Address: 15960, Physical Address: 15960, Value: 255
Logical Address: 20860, Physical Address: 20860, Value: 255
Logical Address: 13425, Physical Address: 13425, Value: 255
Logical Address: 65396, Physical Address: 65396, Value: 255
Logical Address: 20425, Physical Address: 20425, Value: 255
Logical Address: 57261, Physical Address: 57261, Value: 255
Logical Address: 12895, Physical Address: 12895, Value: 255
Logical Address: 55330, Physical Address: 55330, Value: 255
Logical Address: 63030, Physical Address: 63030, Value: 255
Logical Address: 48860, Physical Address: 48860, Value: 255
Logical Address: 15695, Physical Address: 15695, Value: 255
Logical Address: 48595, Physical Address: 48595, Value: 255
Logical Address: 26025, Physical Address: 26025, Value: 255
Logical Address: 61196, Physical Address: 61196, Value: 255
Logical Address: 21825, Physical Address: 21825, Value: 255
Logical Address: 62596, Physical Address: 62596, Value: 255
Logical Address: 27160, Physical Address: 27160, Value: 255
Logical Address: 11325, Physical Address: 11325, Value: 255
Logical Address: 560, Physical Address: 560, Value: 255
Logical Address: 31795, Physical Address: 31795, Value: 255
Logical Address: 31625, Physical Address: 31625, Value: 255
Logical Address: 65131, Physical Address: 65131, Value: 255
Logical Address: 48160, Physical Address: 48160, Value: 255
Logical Address: 4325, Physical Address: 4325, Value: 255
Logical Address: 8695, Physical Address: 8695, Value: 255
Logical Address: 56730, Physical Address: 56730, Value: 255
Logical Address: 50960, Physical Address: 50960, Value: 255
Logical Address: 14995, Physical Address: 14995, Value: 255
Logical Address: 54630, Physical Address: 54630, Value: 255
Logical Address: 51660, Physical Address: 51660, Value: 255
Logical Address: 8960, Physical Address: 8960, Value: 255
Logical Address: 28995, Physical Address: 28995, Value: 255
Logical Address: 38360, Physical Address: 38360, Value: 255
Logical Address: 19195, Physical Address: 19195, Value: 255
Logical Address: 53230, Physical Address: 53230, Value: 255
Logical Address: 63730, Physical Address: 63730, Value: 255
Logical Address: 60230, Physical Address: 60230, Value: 255
Logical Address: 55595, Physical Address: 55595, Value: 255
Logical Address: 35295, Physical Address: 35295, Value: 255
Logical Address: 36260, Physical Address: 36260, Value: 255
Logical Address: 19895, Physical Address: 19895, Value: 255
Logical Address: 47195, Physical Address: 47195, Value: 255
Logical Address: 38095, Physical Address: 38095, Value: 255
Logical Address: 29525, Physical Address: 29525, Value: 255
Logical Address: 295, Physical Address: 295, Value: 255
Logical Address: 59530, Physical Address: 59530, Value: 255
Logical Address: 61630, Physical Address: 61630, Value: 255
Logical Address: 60930, Physical Address: 60930, Value: 255
Logical Address: 49560, Physical Address: 49560, Value: 255
Logical Address: 9660, Physical Address: 9660, Value: 255
Logical Address: 22960, Physical Address: 22960, Value: 255
Logical Address: 12725, Physical Address: 12725, Value: 255
Logical Address: 5895, Physical Address: 5895, Value: 255
Logical Address: 46060, Physical Address: 46060, Value: 255
Logical Address: 5025, Physical Address: 5025, Value: 255
Logical Address: 2660, Physical Address: 2660, Value: 255
Logical Address: 31095, Physical Address: 31095, Value: 255
Logical Address: 37660, Physical Address: 37660, Value: 255
Logical Address: 7825, Physical Address: 7825, Value: 255
Logical Address: 61461, Physical Address: 61461, Value: 255
Logical Address: 11495, Physical Address: 11495, Value: 255
Logical Address: 49995, Physical Address: 49995, Value: 255
Logical Address: 31360, Physical Address: 31360, Value: 255
Logical Address: 9925, Physical Address: 9925, Value: 255
Logical Address: 60761, Physical Address: 60761, Value: 255
Logical Address: 125, Physical Address: 125, Value: 255
Logical Address: 10095, Physical Address: 10095, Value: 255
Logical Address: 44660, Physical Address: 44660, Value: 255
Logical Address: 17095, Physical Address: 17095, Value: 255
Logical Address: 53930, Physical Address: 53930, Value: 255
Logical Address: 57695, Physical Address: 57695, Value: 255
Logical Address: 34595, Physical Address: 34595, Value: 255
Logical Address: 42295, Physical Address: 42295, Value: 255
Logical Address: 28125, Physical Address: 28125, Value: 255
Logical Address: 60496, Physical Address: 60496, Value: 255
Logical Address: 27860, Physical Address: 27860, Value: 255
Logical Address: 22695, Physical Address: 22695, Value: 255
Logical Address: 40460, Physical Address: 40460, Value: 255
Logical Address: 18495, Physical Address: 18495, Value: 255
Logical Address: 41860, Physical Address: 41860, Value: 255
Logical Address: 6425, Physical Address: 6425, Value: 255
Logical Address: 7995, Physical Address: 7995, Value: 255
Logical Address: 45360, Physical Address: 45360, Value: 255
Logical Address: 11060, Physical Address: 11060, Value: 255
Logical Address: 28295, Physical Address: 28295, Value: 255
Logical Address: 44395, Physical Address: 44395, Value: 255
Logical Address: 27425, Physical Address: 27425, Value: 255
Logical Address: 995, Physical Address: 995, Value: 255
Logical Address: 53495, Physical Address: 53495, Value: 255
Logical Address: 35995, Physical Address: 35995, Value: 255
Logical Address: 30225, Physical Address: 30225, Value: 255
Logical Address: 59796, Physical Address: 59796, Value: 255
Logical Address: 33895, Physical Address: 33895, Value: 255
Logical Address: 30925, Physical Address: 30925, Value: 255
Logical Address: 53761, Physical Address: 53761, Value: 255
Logical Address: 8260, Physical Address: 8260, Value: 255
Logical Address: 17625, Physical Address: 17625, Value: 255
Logical Address: 63996, Physical Address: 63996, Value: 255
Logical Address: 32495, Physical Address: 32495, Value: 255
Logical Address: 42995, Physical Address: 42995, Value: 255
Logical Address: 39495, Physical Address: 39495, Value: 255
Logical Address: 34860, Physical Address: 34860, Value: 255
Logical Address: 14560, Physical Address: 14560, Value: 255
Logical Address: 15525, Physical Address: 15525, Value: 255
Logical Address: 64696, Physical Address: 64696, Value: 255
Logical Address: 26460, Physical Address: 26460, Value: 255
Logical Address: 17360, Physical Address: 17360, Value: 255
Logical Address: 26195, Physical Address: 26195, Value: 255
Logical Address: 45095, Physical Address: 45095, Value: 255
Logical Address: 38795, Physical Address: 38795, Value: 255
Logical Address: 40895, Physical Address: 40895, Value: 255
Logical Address: 40195, Physical Address: 40195, Value: 255
Logical Address: 28825, Physical Address: 28825, Value: 255
Logical Address: 54461, Physical Address: 54461, Value: 255
Logical Address: 2225, Physical Address: 2225, Value: 255
Logical Address: 9395, Physical Address: 9395, Value: 255
Logical Address: 50695, Physical Address: 50695, Value: 255
Logical Address: 25325, Physical Address: 25325, Value: 255
Logical Address: 1695, Physical Address: 1695, Value: 255
Logical Address: 47460, Physical Address: 47460, Value: 255
Logical Address: 10360, Physical Address: 10360, Value: 255
Logical Address: 16925, Physical Address: 16925, Value: 255
Logical Address: 4495, Physical Address: 4495, Value: 255
Logical Address: 58130, Physical Address: 58130, Value: 255
Logical Address: 56295, Physical Address: 56295, Value: 255
Logical Address: 29260, Physical Address: 29260, Value: 255
Logical Address: 10625, Physical Address: 10625, Value: 255
Logical Address: 6595, Physical Address: 6595, Value: 255
Logical Address: 57430, Physical Address: 57430, Value: 255
Logical Address: 62330, Physical Address: 62330, Value: 255
Logical Address: 54895, Physical Address: 54895, Value: 255
Logical Address: 23925, Physical Address: 23925, Value: 255
Logical Address: 61896, Physical Address: 61896, Value: 255
Logical Address: 33195, Physical Address: 33195, Value: 255
Logical Address: 36960, Physical Address: 36960, Value: 255
Logical Address: 13860, Physical Address: 13860, Value: 255
Logical Address: 21560, Physical Address: 21560, Value: 255
Logical Address: 24795, Physical Address: 24795, Value: 255
Logical Address: 39760, Physical Address: 39760, Value: 255
Logical Address: 7125, Physical Address: 7125, Value: 255
Logical Address: 1960, Physical Address: 1960, Value: 255
Logical Address: 19725, Physical Address: 19725, Value: 255
Logical Address: 63296, Physical Address: 63296, Value: 255
Logical Address: 21125, Physical Address: 21125, Value: 255
Logical Address: 3095, Physical Address: 3095, Value: 255
Logical Address: 52795, Physical Address: 52795, Value: 255
Logical Address: 24625, Physical Address: 24625, Value: 255
Logical Address: 55861, Physical Address: 55861, Value: 255
Logical Address: 7560, Physical Address: 7560, Value: 255
Logical Address: 23660, Physical Address: 23660, Value: 255
Logical Address: 24095, Physical Address: 24095, Value: 255
Logical Address: 45795, Physical Address: 45795, Value: 255
Logical Address: 32760, Physical Address: 32760, Value: 255
Logical Address: 15260, Physical Address: 15260, Value: 255
Logical Address: 26895, Physical Address: 26895, Value: 255
Logical Address: 39060, Physical Address: 39060, Value: 255
Logical Address: 13160, Physical Address: 13160, Value: 255
Logical Address: 27595, Physical Address: 27595, Value: 255
Logical Address: 33025, Physical Address: 33025, Value: 255
Logical Address: 53061, Physical Address: 53061, Value: 255
Logical Address: 14295, Physical Address: 14295, Value: 255
Logical Address: 43260, Physical Address: 43260, Value: 255
Logical Address: 11760, Physical Address: 11760, Value: 255
Logical Address: 22260, Physical Address: 22260, Value: 255
Logical Address: 18760, Physical Address: 18760, Value: 255
Logical Address: 14125, Physical Address: 14125, Value: 255
Logical Address: 59361, Physical Address: 59361, Value: 255
Logical Address: 12195, Physical Address: 12195, Value: 255
Logical Address: 43960, Physical Address: 43960, Value: 255
Logical Address: 5725, Physical Address: 5725, Value: 255
Logical Address: 62161, Physical Address: 62161, Value: 255
Logical Address: 5460, Physical Address: 5460, Value: 255
Logical Address: 24360, Physical Address: 24360, Value: 255
Logical Address: 18060, Physical Address: 18060, Value: 255
Logical Address: 20160, Physical Address: 20160, Value: 255
Logical Address: 19460, Physical Address: 19460, Value: 255
Logical Address: 25495, Physical Address: 25495, Value: 255
Logical Address: 51130, Physical Address: 51130, Value: 255
Logical Address: 64430, Physical Address: 64430, Value: 255
Logical Address: 54195, Physical Address: 54195, Value: 255
Logical Address: 29960, Physical Address: 29960, Value: 255
Logical Address: 21995, Physical Address: 21995, Value: 255
Logical Address: 46495, Physical Address: 46495, Value: 255
Logical Address: 26725, Physical Address: 26725, Value: 255
Logical Address: 55161, Physical Address: 55161, Value: 255
Logical Address: 13595, Physical Address: 13595, Value: 255
Logical Address: 49295, Physical Address: 49295, Value: 255
Logical Address: 37395, Physical Address: 37395, Value: 255
Logical Address: 35560, Physical Address: 35560, Value: 255
Logical Address: 8525, Physical Address: 8525, Value: 255
Logical Address: 7295, Physical Address: 7295, Value: 255
Logical Address: 51395, Physical Address: 51395, Value: 255
Logical Address: 36695, Physical Address: 36695, Value: 255
Logical Address: 41595, Physical Address: 41595, Value: 255
Logical Address: 34160, Physical Address: 34160, Value: 255
Logical Address: 20595, Physical Address: 20595, Value: 255
Logical Address: 41160, Physical Address: 41160, Value: 255
Logical Address: 12460, Physical Address: 12460, Value: 255
Logical Address: 16225, Physical Address: 16225, Value: 255

```

## Analyze

- TLB Hit Rate Analysis

    - Hit Rate Calculation:
        
        TLB Hit Rate = (Number of TLB Hits) / (Total Number of Accesses)
        TLB Hit Rate = 49 / 1000 = 0.049 or 4.9%

    - Miss Rate Calculation:

        TLB Miss Rate = (Number of TLB Misses) / (Total Number of Accesses)
        TLB Miss Rate = 951 / 1000 = 0.951 or 95.1%

#### Contributing Factors

- Access Pattern: If the accesses are spread out across a wide range of pages or follow a pattern that doesn't revisit the same pages often, the TLB won't be very effective.

- TLB Size: With only 16 entries, the TLB is quite small relative to the number of possible pages (256). Given random or widespread access across the address space, any individual page has a relatively low chance of being in the TLB at any given time.

- Working Set Size: If the working set (the set of pages the application actively uses) is larger than the TLB, it will lead to high miss rates. This is common in scenarios where the application's memory usage pattern is diverse or spread across a large portion of the virtual address space.

## Conclusion

The low TLB hit rate suggests that there's significant room for optimization, either by adjusting the TLB size, modifying access patterns, or exploring different memory management strategies. Analyzing the specific causes of the misses and addressing them through software or simulation parameter changes could lead to meaningful improvements in virtual memory management efficiency.