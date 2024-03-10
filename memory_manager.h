#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#define PAGE_SIZE 256

void translate_and_access_memory(unsigned int logical_address, unsigned char* mem);

#endif // MEMORY_MANAGER_H
