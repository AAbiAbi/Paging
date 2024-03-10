#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

int main() {
    int fd = open("/dev/mem", O_RDONLY);
    if (fd < 0) {
        perror("Failed to open /dev/mem");
        return EXIT_FAILURE;
    }

    // Map a page of physical memory
    off_t offset = 0x100000; // Example physical address
    size_t length = 4096; // Common page size on many systems
    unsigned char* mem = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, offset);
    if (mem == MAP_FAILED) {
        perror("Failed to map memory");
        close(fd);
        return EXIT_FAILURE;
    }

    // Read from the mapped memory
    printf("Data at physical address %p: %02x\n", (void*)offset, mem[0]);

    // Cleanup
    munmap(mem, length);
    close(fd);

    return EXIT_SUCCESS;
}
