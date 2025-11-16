#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

void Debug(char *msg) {
    printf("%s\n", msg);
}

#include "elfloader.c"

int main(int argc, char **argv) {
    printf("name: %s\n", argv[1]);

    FILE *f = fopen(argv[1], "r");
    fseek(f, 0, SEEK_END);
    size_t sz = ftell(f);
    rewind(f);
    printf("sz: %ld\n", sz);

    uint8_t *file_data = malloc(sz);
    fread(file_data, sizeof(uint8_t), sz, f);
    struct elfloader_Result result = elfloader_read(file_data, sz, NULL, 0);
    if (result.error) {
        printf("ERROR: %s\n", result.error);
        return 1;
    }

    printf("OK, memory_size: %lld\n", result.memory_size);

    // Allocate a page aligned chunk of memory
    void *base = malloc(result.memory_size + 4096);
    base       = (void *) (((uint64_t) base + 4095) & ~4095); // page align
    printf("base: %p\n", base);

    struct elfloader_Result result2 = elfloader_read(file_data, sz, base, result.memory_size);
    if (result2.error) {
        printf("ERROR: %s\n", result2.error);
        return 1;
    }
    printf("entrypoint: %p\n", result2.entrypoint);

    // Call the entrypoint
    void (*entry)(void) = (void (*)(void)) result2.entrypoint;
    printf("Calling entrypoint\n");
    entry();
    return 0;
}
