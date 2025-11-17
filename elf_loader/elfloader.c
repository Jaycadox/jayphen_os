#pragma once
#include <stdbool.h>

#ifdef __linux__
#    include <sys/mman.h>
#    include <unistd.h>
#endif

#ifndef NULL
#    define NULL (void *) 0
#endif

#define uint8_t unsigned char
#define uint16_t unsigned short
#define uint32_t unsigned int
#define uint64_t unsigned long long

void Debug(char *msg);

struct elfloader_Seek {
    uint8_t *data;
    size_t   idx, sz;
};

struct elfloader_Result {
    const char *error;       // error message, NULL if success
    uint64_t    memory_size; // size of memory to allocate
    void       *entrypoint;  // pointer to entrypoint function, if the elf was loaded
};

bool elfloader_expect(struct elfloader_Seek *seek, uint8_t *data, size_t n) {
    for (size_t i = seek->idx; i < seek->idx + n && i < seek->sz; ++i) {
        if (seek->data[i] != *data++)
            return false;
    }
    seek->idx += n;
    return true;
}

void elfloader_skipbytes(struct elfloader_Seek *seek, size_t n) {
    if (seek->idx + n > seek->sz) {
        seek->idx = seek->sz;
    } else {
        seek->idx += n;
    }
}

uint8_t elfloader_readbyte(struct elfloader_Seek *seek) {
    if (seek->idx == seek->sz)
        return 0;
    return seek->data[seek->idx++];
}

uint16_t elfloader_readshort(struct elfloader_Seek *seek) {
    if (seek->idx >= seek->sz - sizeof(uint16_t))
        return 0;
    uint16_t x = *(uint16_t *) (seek->data + seek->idx);
    seek->idx += sizeof(uint16_t);
    return x;
}

uint32_t elfloader_readint(struct elfloader_Seek *seek) {
    if (seek->idx >= seek->sz - sizeof(uint32_t))
        return 0;
    uint32_t x = *(uint32_t *) (seek->data + seek->idx);
    seek->idx += sizeof(uint32_t);
    return x;
}

uint64_t elfloader_readlong(struct elfloader_Seek *seek) {
    if (seek->idx >= seek->sz - sizeof(uint64_t))
        return 0;
    uint64_t x = *(uint64_t *) (seek->data + seek->idx);
    seek->idx += sizeof(uint64_t);
    return x;
}

void elfloader_memcpy(void *dest, void *src, size_t n) {
    uint8_t *d = (uint8_t *) dest;
    uint8_t *s = (uint8_t *) src;
    for (size_t i = 0; i < n; ++i) {
        d[i] = s[i];
    }
}

void elfloader_memset(void *dest, uint8_t byte, size_t n) {
    uint8_t *d = (uint8_t *) dest;
    for (size_t i = 0; i < n; ++i) {
        d[i] = byte;
    }
}

void DebugNoop(char *msg) {
    // Do nothing
}

#define Debug DebugNoop

// If memory_base is NULL, then the size of memory to allocate is returned in the elfloader_Result,
// and the elf will not be loaded yet. Call this function again with memory_base pointing to a
// buffer of at least memory_size bytes to actually load the elf.
struct elfloader_Result elfloader_read(uint8_t *data, size_t sz, uint8_t *memory_base, uint64_t memory_size) {
    struct elfloader_Seek seek            = {.data = data, .sz = sz, .idx = 0};
    const char           *error           = NULL;
    uint64_t              entrypoint      = 0;
    uint64_t              new_memory_size = 0;

    // Parse header

    if (!elfloader_expect(&seek, (uint8_t[]) {0x7f, 0x45, 0x4c, 0x46}, 4)) {
        error = "ELF does not start with magic";
        goto done;
    }

    uint8_t bitness = elfloader_readbyte(&seek);
    if (bitness == 1) {
        error = "32 bit ELF is not supported";
        goto done;
    } else if (bitness == 2) {
        Debug("64 bit elf");
    } else {
        error = "invalid bitness";
        goto done;
    }

    uint8_t endianness = elfloader_readbyte(&seek);
    if (endianness == 2) {
        error = "big endian ELF is not supported";
        goto done;
    } else if (endianness == 1) {
        Debug("little endian elf");
    } else {
        error = "invalid endianness";
        goto done;
    }

    uint8_t version = elfloader_readbyte(&seek);
    if (version != 1) {
        error = "invalid version";
        goto done;
    }
    Debug("version 1");

    uint8_t abi = elfloader_readbyte(&seek);
    if (abi != 0) {
        error = "invalid abi";
        goto done;
    }
    Debug("sysV abi");

    /*uint8_t abiversion =*/elfloader_readbyte(&seek);

    // Pad
    elfloader_skipbytes(&seek, 7);

    uint16_t type = elfloader_readshort(&seek);
    if (type != 2) {
        error = "invalid type";
        goto done;
    }
    Debug("position independent executable elf file");

    uint16_t machine = elfloader_readshort(&seek);
    if (machine != 0x3e) {
        error = "invalid machine type";
        goto done;
    }
    Debug("amd64 elf file");

    uint32_t version_2 = elfloader_readint(&seek);
    if (version_2 != 1) {
        error = "invalid version_2";
        goto done;
    }
    Debug("version_2 1");

    entrypoint = elfloader_readlong(&seek);
    Debug("entrypoint");
    //    printf("entrypoint: %llx\n", entrypoint);

    uint64_t program_header_offset = elfloader_readlong(&seek);
    Debug("program header offset");

    /*uint64_t section_header_offset =*/elfloader_readlong(&seek);

    /*uint64_t flags                     = */ elfloader_readint(&seek);
    /*uint64_t header_size               = */ elfloader_readshort(&seek);
    /*uint64_t program_header_entry_size = */ elfloader_readshort(&seek);
    uint64_t program_header_entries = elfloader_readshort(&seek);
    /*uint64_t section_header_entry_size = */ elfloader_readshort(&seek);
    /*uint64_t section_header_entries    = */ elfloader_readshort(&seek);
    /*uint64_t section_name_index        = */ elfloader_readshort(&seek);

    Debug("section header offset");

    //    printf("Num program entries: %lld\n", program_header_entries);
    //    printf("Num section entries: %lld\n", section_header_entries);

    // Parse program header
    struct Program_Header {
        uint64_t offset;
        uint64_t v_address;
        uint64_t file_size;
        uint64_t mem_size;
        uint64_t align;
    };

#define MAX_PROGRAM_HEADERS 32
    struct Program_Header program_headers[MAX_PROGRAM_HEADERS] = {0};
    if (program_header_entries > MAX_PROGRAM_HEADERS) {
        new_memory_size = 0;
        error           = "Too many program headers";
        goto done;
    }

    seek.idx                  = program_header_offset;
    size_t num_loader_headers = 0;
    for (size_t i = 0; i < program_header_entries; ++i) {
        Debug("header");
        uint64_t type = elfloader_readint(&seek);
        if (type != 1) {
            elfloader_skipbytes(&seek, 0x38 - 0x4);
            continue;
        }
        //    printf("type: %lld\n", type);
        /*uint64_t flags =*/elfloader_readint(&seek);
        //    printf("flags: %lld\n", flags);
        program_headers[i].offset    = elfloader_readlong(&seek);
        program_headers[i].v_address = elfloader_readlong(&seek);
        /*uint64_t p_address           =*/elfloader_readlong(&seek);
        program_headers[i].file_size = elfloader_readlong(&seek);
        program_headers[i].mem_size  = elfloader_readlong(&seek);
        program_headers[i].align     = elfloader_readlong(&seek);
        num_loader_headers++;
    }

    // Find smallest v_address
    uint64_t min_vaddr = 0xffffffffffffffff;
    for (size_t i = 0; i < num_loader_headers; ++i) {
        if (program_headers[i].v_address < min_vaddr) {
            min_vaddr = program_headers[i].v_address;
        }
    }
    //    printf("min_vaddr: %llx\n", min_vaddr);

    // Subtract min_vaddr from all v_addresses
    for (size_t i = 0; i < num_loader_headers; ++i) {
        program_headers[i].v_address -= min_vaddr;
    }
    entrypoint -= min_vaddr;

    // Print debug info
    //    for (size_t i = 0; i < num_loader_headers; ++i) {
    //        printf("Program Header %zu:\n", i);
    //        printf("offset: %llx\n", program_headers[i].offset);
    //        printf("v_address: %llx\n", program_headers[i].v_address);
    //        printf("file_size: %llx\n", program_headers[i].file_size);
    //        printf("mem_size: %llx\n", program_headers[i].mem_size);
    //        printf("align: %llx\n", program_headers[i].align);
    //        printf("\n");
    //    }

    // Calculate memory size
    for (size_t i = 0; i < num_loader_headers; ++i) {
        uint64_t end = program_headers[i].v_address + program_headers[i].mem_size;
        if (end > new_memory_size) {
            new_memory_size = end;
        }
    }

    if (!memory_base) {
        memory_size = new_memory_size;
        //    printf("memory_size: %llx\n", memory_size);
        goto done;
    }

    if (memory_size < new_memory_size) {
        error = "memory size too small";
        goto done;
    }

    for (size_t i = 0; i < num_loader_headers; ++i) {
        uint64_t dest = (uint64_t) memory_base + program_headers[i].v_address;
        elfloader_memcpy((void *) dest, data + program_headers[i].offset, program_headers[i].file_size);
        if (program_headers[i].file_size < program_headers[i].mem_size) {
            elfloader_memset((void *) (dest + program_headers[i].file_size), 0, program_headers[i].mem_size - program_headers[i].file_size);
        }

#ifdef __linux__
        mprotect((void *) dest, program_headers[i].mem_size, PROT_READ | PROT_WRITE | PROT_EXEC);
#endif
        //    printf("Loaded %llx bytes to %p\n", program_headers[i].file_size, (void*)dest);
    }
    entrypoint += (uint64_t) memory_base;

done: {
    struct elfloader_Result ret = (struct elfloader_Result) {.error = error, .memory_size = new_memory_size, .entrypoint = (void *) entrypoint};
    return ret;
}
}

#undef Debug
