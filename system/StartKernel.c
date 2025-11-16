#include "elf_libc.c"

int main(void) {
    struct MemoryLayout    *Layout      = GetMemoryLayout();
    struct FrameBufferInfo *FrameBuffer = GetFrameBufferInfo();
    ExitUEFIMode();
    gFrameBuffer = FrameBuffer;

    for (int x = 0; x < FrameBuffer->Width; ++x) {
        for (int y = 0; y < FrameBuffer->Height; ++y) {
            FrameBuffer->Base[y * FrameBuffer->Width + x].Blue  = 0x00;
            FrameBuffer->Base[y * FrameBuffer->Width + x].Green = 0x00;
            FrameBuffer->Base[y * FrameBuffer->Width + x].Red =
                0x44 * ((float) x / (float) FrameBuffer->Width);
        }
    }
    PrintLinef("Boot info:");
    PrintLinef("Num. memory mapped IO regions: %zu", Layout->NumMemoryMappedIORegions);
    PrintLinef("Num. port mapped IO regions  : %zu", Layout->NumPortMappedIORegions);
    PrintLinef("Num. conventional regions    : %zu", Layout->NumConventionalRegions);
    PrintLinef("Num. ACPI regions            : %zu", Layout->NumACPIRegions);
    PrintLinef("Framebuffer: %dx%d @ %p (%zu bytes)",
               FrameBuffer->Width,
               FrameBuffer->Height,
               FrameBuffer->Base,
               FrameBuffer->Size);

    for (int i = 0; i < 200; ++i) {
        // PrintLinef("Hello, world! %d", i);
    }
    for (;;)
        ;
    return 0;
}
