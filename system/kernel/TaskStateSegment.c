#pragma once
#include "../elf_libc.c"

struct TaskStateSegmentEntry {
    u32 Reserved;
    u64 RingStackPointers[3];
    u64 Reserved2;
    u64 InterruptStackTables[7];
    u64 Reserved3;
    u16 IOMapBaseAddress;
    u16 Reserved4;
} __attribute__((packed));
_Static_assert(sizeof(struct TaskStateSegmentEntry) == 0x68, "TaskStateSegmentEntry wrong size");

static struct TaskStateSegmentEntry gTSSEntry = {0};

struct TaskStateSegmentEntry *CreateTaskStateSegmentEntry(void *KernelStackStart) {
    memset(&gTSSEntry, 0, sizeof(struct TaskStateSegmentEntry));

    gTSSEntry.RingStackPointers[0]    = (u64) (usize) KernelStackStart;
    gTSSEntry.InterruptStackTables[0] = (u64) (usize) KernelStackStart;
    gTSSEntry.IOMapBaseAddress        = sizeof(struct TaskStateSegmentEntry);

    return &gTSSEntry;
}

void LoadTaskStateSegmentEntry(void) {
    __asm__ volatile("ltr %0" ::"r"((u16) 0x18));
}
