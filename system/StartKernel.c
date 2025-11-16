#include "elf_libc.c"

int main(void) {
    struct MemoryLayout *Layout = GetMemoryLayout();
    PrintLinef("Num. memory mapped IO regions: %zu", Layout->NumMemoryMappedIORegions);
    PrintLinef("Num. port mapped IO regions  : %zu", Layout->NumPortMappedIORegions);
    PrintLinef("Num. conventional regions    : %zu", Layout->NumConventionalRegions);
    PrintLinef("Num. ACPI regions            : %zu", Layout->NumACPIRegions);

    PrintLine("Booting...");
    for (;;)
        ;
    return 0;
}
