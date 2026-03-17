global _start
extern kmain

section .bss
align 16
kernel_stack_bottom:
    resb 16384
kernel_stack_top:

section .text
_start:
    mov rsp, kernel_stack_top
    mov rbp, rsp

    extern kmain
    call kmain
    cli
    hlt
    jmp $