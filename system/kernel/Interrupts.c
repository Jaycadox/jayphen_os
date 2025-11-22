#pragma once
#include "../elf_libc.c"
#include "InterruptDescriptorTable.c"

struct CPUInterruptFrame {
    u64 Rip, Cs, RFlags, Rsp, Ss;
};

struct InterruptFrame {
    u64 Rax, Rcx, Rdx, Rbx, Rbp, Rsi, Rdi, R8, R9, R10, R11, R12, R13, R14, R15;
    u64 InterruptNumber;
    u64 Error;

    // Pushed automatically by the CPU
    struct CPUInterruptFrame Frame;
};

void ExceptionHandler(struct InterruptFrame *Frame) {
    switch (Frame->InterruptNumber) {
    case FAULT_DIV_ERROR:
        Panic("Divide by zero!");
    case ABORT_DOUBLE_FAULT:
        Panic("Double fault!");
    case FAULT_PAGE:
        Panic("Page fault!");
    case FAULT_GENERAL_PROTECTION:
        Panic("General protection fault!");
    default:
        Panic("not Divide by zero!");
    };
}

void IRQHandler(struct InterruptFrame *Frame) {
    // DebugLinef("hello?");
    // for(;;);
    OutByte(0x20, 0x20);
    TickCount += 1;
}

__attribute__((naked)) void InterruptCommon(void) {
    __asm__ volatile("pushq %r15\n\t"
                     "pushq %r14\n\t"
                     "pushq %r13\n\t"
                     "pushq %r12\n\t"
                     "pushq %r11\n\t"
                     "pushq %r10\n\t"
                     "pushq %r9\n\t"
                     "pushq %r8\n\t"
                     "pushq %rdi\n\t"
                     "pushq %rsi\n\t"
                     "pushq %rbp\n\t"
                     "pushq %rbx\n\t"
                     "pushq %rdx\n\t"
                     "pushq %rcx\n\t"
                     "pushq %rax\n\t"

                     "movq %rsp, %rdi\n\t"
                     "call ExceptionHandler\n\t"

                     "popq %rax\n\t"
                     "popq %rcx\n\t"
                     "popq %rdx\n\t"
                     "popq %rbx\n\t"
                     "popq %rbp\n\t"
                     "popq %rsi\n\t"
                     "popq %rdi\n\t"
                     "popq %r8\n\t"
                     "popq %r9\n\t"
                     "popq %r10\n\t"
                     "popq %r11\n\t"
                     "popq %r12\n\t"
                     "popq %r13\n\t"
                     "popq %r14\n\t"
                     "popq %r15\n\t"

                     "addq $16, %rsp\n\t"
                     "iretq");
}

__attribute__((naked)) void isr0(void) {
    __asm__ volatile("pushq $0\n\tpushq $0\n\tjmp InterruptCommon");
}

__attribute__((naked)) void isr8(void) {
    __asm__ volatile("pushq $8\n\tjmp InterruptCommon");
}

__attribute__((naked)) void isr13(void) {
    __asm__ volatile("pushq $13\n\tjmp InterruptCommon");
}

__attribute__((naked)) void isr14(void) {
    __asm__ volatile("pushq $14\n\tjmp InterruptCommon");
}

__attribute__((naked)) void IRQRequest(void) {
    __asm__ volatile("pushq %r15\n\t"
                     "pushq %r14\n\t"
                     "pushq %r13\n\t"
                     "pushq %r12\n\t"
                     "pushq %r11\n\t"
                     "pushq %r10\n\t"
                     "pushq %r9\n\t"
                     "pushq %r8\n\t"
                     "pushq %rdi\n\t"
                     "pushq %rsi\n\t"
                     "pushq %rbp\n\t"
                     "pushq %rbx\n\t"
                     "pushq %rdx\n\t"
                     "pushq %rcx\n\t"
                     "pushq %rax\n\t"

                     "movq %rsp, %rdi\n\t"
                     "call IRQHandler\n\t"

                     "popq %rax\n\t"
                     "popq %rcx\n\t"
                     "popq %rdx\n\t"
                     "popq %rbx\n\t"
                     "popq %rbp\n\t"
                     "popq %rsi\n\t"
                     "popq %rdi\n\t"
                     "popq %r8\n\t"
                     "popq %r9\n\t"
                     "popq %r10\n\t"
                     "popq %r11\n\t"
                     "popq %r12\n\t"
                     "popq %r13\n\t"
                     "popq %r14\n\t"
                     "popq %r15\n\t"

                     "addq $16, %rsp\n\t"
                     "iretq");
}

__attribute__((naked)) void irq0(void) {
    __asm__ volatile("pushq $0\n\t"
                     "pushq $32\n\t"
                     "jmp IRQRequest\n\t");
}
