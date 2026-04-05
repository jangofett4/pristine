; Pristine
; stage2_isr - ISR stubs
; SPDX-License-Identifier: MIT

extern idt32_isr_handler

%macro ISR_NOERR 1
global idt32_isr_%1
idt32_isr_%1:
    push dword 0
    push dword %1
    jmp isr_common
%endmacro

%macro ISR_ERR 1
global idt32_isr_%1
idt32_isr_%1:
    push dword %1
    jmp isr_common
%endmacro

isr_common:
    pusha
    push ds
    push es
    push fs
    push gs
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov eax, esp    ; save esp before push
    push eax        ; push frame pointer
    call idt32_isr_handler
    add esp, 4
    pop gs
    pop fs
    pop es
    pop ds
    popa
    add esp, 8
    iret

global isr_default
isr_default:
    iret

global idt32_load_idtr
idt32_load_idtr:
    mov eax, [esp+4]
    lidt [eax]
    ret

ISR_NOERR 0
ISR_NOERR 1
ISR_NOERR 2
ISR_NOERR 3
ISR_NOERR 4
ISR_NOERR 5
ISR_NOERR 6
ISR_NOERR 7
ISR_ERR   8     ; Double fault
ISR_NOERR 9
ISR_ERR   10    ; Invalid TSS
ISR_ERR   11    ; Segment not present
ISR_ERR   12    ; Stack fault
ISR_ERR   13    ; General protection fault
ISR_ERR   14    ; Page fault
ISR_NOERR 15
ISR_NOERR 16
ISR_ERR   17    ; Alignment check
ISR_NOERR 18
ISR_NOERR 19
ISR_NOERR 20
ISR_ERR   21
ISR_NOERR 22
ISR_NOERR 23
ISR_NOERR 24
ISR_NOERR 25
ISR_NOERR 26
ISR_NOERR 27
ISR_NOERR 28
ISR_NOERR 29
ISR_ERR   30    ; Security exception
ISR_NOERR 31
ISR_NOERR 32
ISR_NOERR 33
ISR_NOERR 34
ISR_NOERR 35
ISR_NOERR 36
ISR_NOERR 37
ISR_NOERR 38
ISR_NOERR 39
ISR_NOERR 40
ISR_NOERR 41
ISR_NOERR 42
ISR_NOERR 43
ISR_NOERR 44
ISR_NOERR 45
ISR_NOERR 46
ISR_NOERR 47