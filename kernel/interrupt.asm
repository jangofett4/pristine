; Pristine
; interrupt: common ISR handler definitions
; SPDX-License-Identifier: MIT

extern idt64_isr_handler

%macro ISR_NOERR 1
global idt64_isr_%1
idt64_isr_%1:
    push qword 0
    push qword %1
    jmp isr_common
%endmacro

%macro PIC_ISR_NOERR 1
global pic_isr_%1
pic_isr_%1:
    push qword 0
    push qword %1
    jmp isr_common
%endmacro

%macro ISR_ERR 1
global idt64_isr_%1
idt64_isr_%1:
    push qword %1
    jmp isr_common
%endmacro

isr_common:
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
    mov rdi, rsp ; System V ABI frame pointer
    call idt64_isr_handler
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
    add rsp, 16 ; clean up error code + interrupt number pushed by stub
    iretq

global isr_default
isr_default:
    iret

global idt64_load_idtr
idt64_load_idtr:
    lidt [rdi]
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

; PIC
PIC_ISR_NOERR 32
PIC_ISR_NOERR 33
PIC_ISR_NOERR 34
PIC_ISR_NOERR 35
PIC_ISR_NOERR 36
PIC_ISR_NOERR 37
PIC_ISR_NOERR 38
PIC_ISR_NOERR 39
PIC_ISR_NOERR 40
PIC_ISR_NOERR 41
PIC_ISR_NOERR 42
PIC_ISR_NOERR 43
PIC_ISR_NOERR 44
PIC_ISR_NOERR 45
PIC_ISR_NOERR 46
PIC_ISR_NOERR 47