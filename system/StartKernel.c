#define KERNEL_DEBUG
#define KERNEL_STACK_PAGES 256
#include "elf_libc.c"

#include "./kernel/Allocator.c"
#include "./kernel/GlobalDescriptorTable.c"
#include "./kernel/ProgrammableIntervalTimer.c"
#include "./kernel/PCI.c"

int main(void) {
    struct MemoryLayout    *Layout      = GetMemoryLayout();
    struct FrameBufferInfo *FrameBuffer = GetFrameBufferInfo();
    ExitUEFIMode();
    gFrameBuffer = FrameBuffer;

    for (int x = 0; x < FrameBuffer->Width; ++x) {
        for (int y = 0; y < FrameBuffer->Height; ++y) {
            FrameBuffer->Base[y * FrameBuffer->Width + x].Blue  = 0x00;
            FrameBuffer->Base[y * FrameBuffer->Width + x].Green = 0x00;
            FrameBuffer->Base[y * FrameBuffer->Width + x].Red   = 0xFF * ((float) x / (float) FrameBuffer->Width);
        }
    }
    DebugLinef("Boot info:");
    DebugLinef("Num. memory mapped IO regions: %zu", Layout->NumMemoryMappedIORegions);
    DebugLinef("Num. port mapped IO regions  : %zu", Layout->NumPortMappedIORegions);
    DebugLinef("Num. conventional regions    : %zu", Layout->NumConventionalRegions);
    DebugLinef("Num. ACPI regions            : %zu", Layout->NumACPIRegions);
    DebugLinef("Framebuffer: %dx%d @ %p (%zu bytes)", FrameBuffer->Width, FrameBuffer->Height, FrameBuffer->Base, FrameBuffer->Size);

    InitializeAllocator(Layout);
    void *KernelStackStart = AllocatePages(KERNEL_STACK_PAGES);
    InitializeGlobalDescriptorTable(KernelStackStart + (KERNEL_STACK_PAGES * PAGE_SIZE));
    EnableProgrammableIntervalTimer(1000);
    ScanPCIBus();

    for (;;)
    {
        // PrintLinef("test");
    }

    return 0;
}
