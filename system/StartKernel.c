// #define KERNEL_DEBUG
#define KERNEL_STACK_PAGES 256
#include "elf_libc.c"

#include "kernel/Allocator.c"
#include "kernel/GlobalDescriptorTable.c"
#include "kernel/Input.c"
#include "kernel/PCI.c"
#include "kernel/ProgrammableIntervalTimer.c"

int main(void) {
    struct MemoryLayout    *Layout      = GetMemoryLayout();
    struct FrameBufferInfo *FrameBuffer = GetFrameBufferInfo();
    ExitUEFIMode();
    gFrameBuffer = FrameBuffer;
    for (u32 i = 0; i < FrameBuffer->LineNumber + 3; ++i) {
        PrintLine("");
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

    PrintLine("\njayphenOS alpha\n");

    char Line[256] = {0};
    for (;;) {
        Print(" $ ");
        usize Size = InputReadLine(Line, sizeof(Line));
        if (Size != 0) {
            if (StrEq(Line, "reset")) {
                TriggerTripleFault();
            } else if (StrEq(Line, "panic")) {
                Panic("User triggered panic");
            } else if (StrEq(Line, "clear")) {
                TerminalClear();
            } else if (StrEq(Line, "help") || StrEq(Line, "?")) {
                PrintLine("help/?: shows this dialoge");
                PrintLine("reset : resets the system");
                PrintLine("panic : panics the system");
                PrintLine("clear : clears the screen");
                PrintLine("...   : echoes prompt");
            } else {
                PrintLinef(">> %s", Line);
            }
        }
    }
    return 0;
}
