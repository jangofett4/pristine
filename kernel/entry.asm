global _start
extern kmain

extern __kernel_stack_start

section .text
_start:
    mov rsp, __kernel_stack_start
    mov rbp, rsp

    extern kmain
    call kmain
    cli
    hlt
    jmp $