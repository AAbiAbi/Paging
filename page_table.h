#ifndef PAGE_TABLE_H
#define PAGE_TABLE_H

#include "tlb.h"

#define NUM_PAGES 256
#define FRAME_NUMBER_NOT_FOUND -1

// Function declarations
void initialize_page_table();
int get_frame_number_from_page_table(unsigned int page_number);
void update_page_table(unsigned int page_number, int frame_number);

#endif // PAGE_TABLE_H
