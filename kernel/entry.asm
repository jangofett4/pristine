global _start
extern kmain

extern __kernel_stack_top

section .text
_start:
    mov rsp, __kernel_stack_top
    mov rbp, rsp

    extern kmain
    call kmain
    cli
    hlt
    jmp $