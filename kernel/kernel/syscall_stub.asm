; Pristine
; syscall_stub: syscall stub
; SPDX-License-Identifier MIT
[BITS 64]

DEFAULT REL

extern syscall_table
extern syscall_table_count

global syscall_stub
syscall_stub:
    swapgs
    mov [gs:0x10], rsp ; Save user RSP
    mov rsp, [gs:0x08] ; Move to kernel stack

    push rcx ; User RIP
    push r11 ; User RFLAGS
    push rbp 

    cmp rax, [syscall_table_count]
    jae .invalid

    shl rax, 3
    call [syscall_table+rax]
    jmp .done

.invalid:
    mov rax, -1

.done:
    pop rbp
    pop r11
    pop rcx

    mov rsp, [gs:0x10] ; Restore user RSP
    swapgs
    o64 sysret