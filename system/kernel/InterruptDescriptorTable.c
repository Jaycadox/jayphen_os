#pragma once
#include "../elf_libc.c"

struct InterruptDescriptorTableLayout {
    u16                                   Size; // sizeof(IDT) - 1, in bytes
    struct InterruptDescriptorTableEntry *Entries;
} __attribute__((packed));
_Static_assert(sizeof(struct InterruptDescriptorTableLayout) == 10, "IDT layout wrong size");

struct InterruptDescriptorTableEntry {
    u16 Offset1;
    u16 Selector;
    u8  InterruptStackTableIndex;
    u8  Flags;
    u16 Offset2;
    u32 Offset3;
    u32 Zero;
} __attribute__((packed));
_Static_assert(sizeof(struct InterruptDescriptorTableEntry) == 16, "IDT entry wrong size");

#define MAX_IDT_ENTRIES 256
static struct InterruptDescriptorTableEntry  gIDTEntries[MAX_IDT_ENTRIES]              = {0};
static struct InterruptDescriptorTableLayout gIDTLayout                                = {0};
static u16                                   gKernelCodeIDTSelector;

enum InterruptDescriptorTableFlag {
    IDT_GATE_TYPE_INTERRUPT = 0b1110,
    IDT_GATE_TYPE_TRAP      = 0b1110,

    IDT_PRIVILEGE_RING0 = 0b000000,
    IDT_PRIVILEGE_RING3 = 0b110000,

    IDT_PRESENT = 0b10000000,
};

enum InterruptType {
    FAULT_DIV_ERROR = 0x00,
    TRAP_DEBUG,
    INTERRUPT_NMI,
    TRAP_BREAKPOINT,
    TRAP_OVERFLOW,
    FAULT_BOUND_RANGE_EXCEEDED,
    FAULT_INVALID_OPCODE,
    FAILT_NO_FP,
    ABORT_DOUBLE_FAULT,
    FAULT_FP_OVERRUN,
    FAULT_INVALID_TSS,
    FAULT_SEGMENT_NOT_PRESENT,
    FAILT_STACK_SEGMENT,
    FAULT_GENERAL_PROTECTION,
    FAULT_PAGE,
    FAULT_RESERVED,
    FAULT_FPERROR,
    FAULT_ALIGNMENT_CHECK,
    ABORT_MACHINE_CHECK,
    FAULT_SIMD,
    FAULT_VIRTUALIZATION,
    FAULT_CONTROL_PROTECTION,

    INTERRUPT_IRQ0 = 32,
};

void SetInterruptDescriptorTableEntry(enum InterruptType Type, u16 IDTSelector, u8 Flags, void *Handler, bool UseISTStack) {
    if (Type >= MAX_IDT_ENTRIES) {
        Panic("IDT entry overflow");
    }

    gIDTEntries[Type].Offset1                  = (usize) Handler & 0xFFFF;
    gIDTEntries[Type].Selector                 = IDTSelector;
    u8 IST = UseISTStack ? 1 : 0;
    gIDTEntries[Type].InterruptStackTableIndex = UseISTStack ? 1 : 0;
    gIDTEntries[Type].Flags                    = Flags;
    gIDTEntries[Type].Offset2                  = ((usize) Handler >> 16) & 0xFFFF;
    gIDTEntries[Type].Offset3                  = ((usize) Handler >> 32) & 0xFFFFFFFF;
    gIDTEntries[Type].Zero                     = 0;

    DebugLinef("+IDT Entry [%zu/%d]: Selector = %X, Flags = %X, Handler = %p", (usize) Type, MAX_IDT_ENTRIES, IDTSelector, Flags, Handler);
}

#include "./Interrupts.c"

void UnmaskPIC(u8 IRQNumber) {
    if (IRQNumber >= 8) {
        u8 mask = InByte(0xA1);
        OutByte(0xA1, mask & ~(1 << (IRQNumber - 8)));
        mask = InByte(0x21);
        OutByte(0x21, mask & ~(1 << 2));
    } else {
        u8 mask = InByte(0x21);
        OutByte(0x21, mask & ~(1 << IRQNumber));
    }
}

void SetDriverInterruptDescriptorTableEntry(u8 IRQNumber, void *Handler) {
    u8 Flags = IDT_GATE_TYPE_INTERRUPT | IDT_PRIVILEGE_RING0 | IDT_PRESENT;
    SetInterruptDescriptorTableEntry(INTERRUPT_IRQ0 + IRQNumber, gKernelCodeIDTSelector, Flags, Handler, false);
    UnmaskPIC(IRQNumber);
}

void InitializeInterruptDescriptorTable(u16 KernelCodeIDTSelector) {
    gKernelCodeIDTSelector = KernelCodeIDTSelector;
    gIDTLayout.Size    = sizeof(gIDTEntries) - 1;
    gIDTLayout.Entries = gIDTEntries;

    MemSet(&gIDTEntries, 0, sizeof(gIDTEntries));

    u8 Flags = IDT_GATE_TYPE_INTERRUPT | IDT_PRIVILEGE_RING0 | IDT_PRESENT;
    SetInterruptDescriptorTableEntry(FAULT_DIV_ERROR, KernelCodeIDTSelector, Flags, isr0, false);
    SetInterruptDescriptorTableEntry(ABORT_DOUBLE_FAULT, KernelCodeIDTSelector, Flags, isr8, true);
    SetInterruptDescriptorTableEntry(FAULT_GENERAL_PROTECTION, KernelCodeIDTSelector, Flags, isr13, false);
    SetInterruptDescriptorTableEntry(FAULT_PAGE, KernelCodeIDTSelector, Flags, isr14, false);
    SetInterruptDescriptorTableEntry(INTERRUPT_IRQ0, KernelCodeIDTSelector, Flags, irq0, false);

    __asm__ __volatile__("lidt %0" : : "m"(gIDTLayout));
    __asm__ __volatile__("sti"); // Enable interrupts

    DebugLinef("Loaded IDT entries");
}
