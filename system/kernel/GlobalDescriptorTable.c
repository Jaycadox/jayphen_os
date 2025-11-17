#pragma once
#include "../elf_libc.c"
#include "./Allocator.c"
#include "TaskStateSegment.c"

struct GlobalDescriptorTableLayout {
    u16                                Size; // sizeof(GDT) - 1, in bytes
    struct GlobalDescriptorTableEntry *Entries;
} __attribute__((packed));
_Static_assert(sizeof(struct GlobalDescriptorTableLayout) == 10, "GDT layout wrong size");

struct GlobalDescriptorTableEntry {
    u16 Limit;
    u16 Base1;
    u8  Base2;
    u8  Access;
    u8  FlagsLimit;
    u8  Base3;
} __attribute__((packed));
_Static_assert(sizeof(struct GlobalDescriptorTableEntry) == 8, "GDT entry wrong size");

#define MAX_GDT_ENTRIES 5
static usize                              gGDTCurrentEntryIndex        = 0;
static struct GlobalDescriptorTableEntry  gGDTEntries[MAX_GDT_ENTRIES] = {0};
static struct GlobalDescriptorTableLayout gGDTLayout                   = {0};

struct GlobalDescriptorTableEntry MakeGlobalDescriptorTableEntry(u32 Base, u32 Limit, u8 Access, u8 Flags) {
    struct GlobalDescriptorTableEntry Entry = {0};

    Entry.Base1      = Base & 0xFFFF;
    Entry.Base2      = (Base >> 16) & 0xFF;
    Entry.Base3      = (Base >> 24) & 0xFF;
    Entry.Limit      = Limit & 0xFFFF;
    Entry.FlagsLimit = ((Limit >> 16) & 0x0F) | (Flags & 0xF0);
    Entry.Access     = Access;

    return Entry;
}

void PushGlobalDescriptorTableEntry(u32 Base, u32 Limit, u8 Access, u8 Flags) {
    usize Index = gGDTCurrentEntryIndex++;
    if (Index >= MAX_GDT_ENTRIES) {
        Panic("GDT entry overflow");
    }
    struct GlobalDescriptorTableEntry Entry = MakeGlobalDescriptorTableEntry(Base, Limit, Access, Flags);
    gGDTEntries[Index]                      = Entry;

    DebugLinef("+GDT Entry [%zu/%d]: Base = %X, Limit = %X, Access = %X, Flags = %X", Index + 1, MAX_GDT_ENTRIES, Base, Limit, Access, Flags);
}

void LoadGlobalDescriptorTable(void) {
    gGDTLayout.Entries = gGDTEntries;
    gGDTLayout.Size    = sizeof(gGDTEntries) - 1;

    struct GlobalDescriptorTableLayout *Layout = &gGDTLayout;
    __asm__ volatile("lgdt  (%0)        \n\t"      // Give CPU a pointer to the GDT layout
                     "mov   $0x10, %%ax \n\t"      // 0x10=16 AKA 3rd entry into GDT (kernel data entry)
                     "mov   %%ax, %%ds  \n\t"      // Move kernel code entry into into data segment
                     "mov   %%ax, %%es  \n\t"      // Move kernel code entry into extra segment
                     "mov   %%ax, %%fs  \n\t"      // Move kernel code entry into extra segment
                     "mov   %%ax, %%gs  \n\t"      // Move kernel code entry into extra segment
                     "mov   %%ax, %%ss  \n\t"      // Move kernel code entry into stack segment
                     "pushq $0x08       \n\t"      // Push 2nd entry of GDT (kernel code segment)
                     "lea   1f(%%rip), %%rax \n\t" // Set code segment register to kernel code segment (roundabout way)
                     "pushq %%rax       \n\t"
                     "lretq             \n\t"
                     "1:                \n\t"
                     :
                     : "r"(Layout) // input 0
                     : "rax", "memory");

    DebugLinef("Loaded GDT entries");
}

enum GlobalDescriptorTableAccess {
    GDT_PRESENT = 0b10000000,

    GDT_RING0 = 0b00000000,
    GDT_RING1 = 0b00100000,
    GDT_RING2 = 0b01000000,

    GDT_DESCRIPTOR_CODE_OR_DATA       = 0b00010000,
    GDT_DESCRIPTOR_TASK_STATE_SEGMENT = 0b00000000,

    GDT_EXECUTABLE     = 0b00001000,
    GDT_SYSTEM         = 0b00001000,
    GDT_NOT_EXECUTABLE = 0b00000000,

    GDT_DATA_DIRECTION_DOWN           = 0b00000100,
    GDT_DATA_DIRECTION_UP             = 0b00000000,
    GDT_CODE_PRIVILEGE_EQUAL_OR_LOWER = 0b00000100,
    GDT_CODE_PRIVILEGE_EQUAL          = 0b00000000,

    GDT_CODE_DISALLOW_READS  = 0b00000000,
    GDT_CODE_ALLOW_READS     = 0b00000010,
    GDT_DATA_DISALLOW_WRITES = 0b00000000,
    GDT_DATA_ALLOW_WRITES    = 0b00000010,

    GDT_ACCESS_UNSET = 0b00000000,
    GDT_ACCESS_SET   = 0b00000001,
};

enum GlobalDescriptorTableFlags {
    GDT_FLAG_BYTES = 0b00000000,
    GDT_FLAG_PAGES = 0b10000000,

    GDT_FLAG_16BIT_PROTECTED_MODE = 0b00000000,
    GDT_FLAG_32BIT_PROTECTED_MODE = 0b01000000,
    GDT_FLAG_64BIT_PROTECTED_MODE = 0b00100000,
};

void PushTaskStateSegmentEntries(struct TaskStateSegmentEntry *Entry) {
    usize Base  = (usize) Entry;
    usize Limit = sizeof(struct TaskStateSegmentEntry) - 1;
    // Lower entry
    PushGlobalDescriptorTableEntry(Base & 0xFFFFFFFF,
                                   Limit,
                                   GDT_PRESENT | GDT_RING0 | GDT_DESCRIPTOR_TASK_STATE_SEGMENT | GDT_SYSTEM | GDT_CODE_PRIVILEGE_EQUAL |
                                       GDT_CODE_DISALLOW_READS | GDT_ACCESS_SET,
                                   0);
    // Higher entry
    PushGlobalDescriptorTableEntry(Base >> 32, 0, 0, 0);
}

void InitializeGlobalDescriptorTable(void *KernelStackStart) {
    // NULL descriptor
    PushGlobalDescriptorTableEntry(0, 0, 0, 0);

    // Kernel code segment
    PushGlobalDescriptorTableEntry(0,       // Base, ignored in 64 bit mode
                                   0xFFFFF, // Limit, ignored in 64 bit mode
                                   GDT_PRESENT | GDT_RING0 | GDT_DESCRIPTOR_CODE_OR_DATA | GDT_EXECUTABLE | GDT_CODE_PRIVILEGE_EQUAL |
                                       GDT_CODE_ALLOW_READS,                        // Access
                                   GDT_FLAG_PAGES | GDT_FLAG_64BIT_PROTECTED_MODE); // Flags

    // Kernel data segment
    PushGlobalDescriptorTableEntry(0,       // Base, ignored in 64 bit mode
                                   0xFFFFF, // Limit, ignored in 64 bit mode
                                   GDT_PRESENT | GDT_RING0 | GDT_DESCRIPTOR_CODE_OR_DATA | GDT_NOT_EXECUTABLE | GDT_DATA_DIRECTION_UP |
                                       GDT_CODE_ALLOW_READS,                        // Access
                                   GDT_FLAG_PAGES | GDT_FLAG_64BIT_PROTECTED_MODE); // Flags

    PushTaskStateSegmentEntries(CreateTaskStateSegmentEntry(KernelStackStart));

    LoadGlobalDescriptorTable();
    LoadTaskStateSegmentEntry();

    DebugLinef("TSS kernel stack start: %p (256 pages)", KernelStackStart);
}
