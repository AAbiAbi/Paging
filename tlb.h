#ifndef TLB_H
#define TLB_H

#define TLB_ENTRIES 16
#include <time.h> // Include this to define time_t

// TLB entry structure
typedef struct {
    unsigned int page_number;
    int frame_number;
    time_t last_accessed;
    int valid;
} TLBEntry;

// Function declarations
void initialize_tlb();
int consult_tlb(unsigned int page_number);
void update_tlb(unsigned int page_number, int frame_number);

#endif // TLB_H
