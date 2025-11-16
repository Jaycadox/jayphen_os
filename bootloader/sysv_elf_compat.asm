default rel
bits 64

global CallSysVEntrypoint
global SystemInvoke
extern SystemDispatcher

section .text

SystemInvoke:
    push rdi
    push rsi
    push rbx
    push rbp
    push r12
    push r13
    push r14
    push r15

    mov rcx, rdi
    mov rdx, rsi

    sub rsp, 40

    call SystemDispatcher

    add rsp, 40
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    pop rbx
    pop rsi
    pop rdi
    ret

CallSysVEntrypoint:
    push rdi
    push rsi
    mov rdi, rdx
    call rcx
    pop rsi
    pop rdi
    ret