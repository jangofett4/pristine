; Pristine
; stage1 - stage 1 bootloader, responsible for loading up stage 2 into memory, loading GDT, enabling 32 bit & jumping to stage 2 bootloader
; SPDX-License-Identifier: MIT

[bits 16]
extern puts
global stage1_boot
section .text
stage1_boot:
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    mov [boot_drive], dl 

    call stage1_video_clear
    call stage1_reset_cursor

    call stage1_readdisk

    cli
    lgdt [stage1_gdt_ptr]
    mov eax, cr0
    or eax, 0b1
    mov cr0, eax

    jmp 0x08:.stage1_farjump

    mov si, boot_message_lgdt_panic
    call stage1_video_puts

    jmp $

[bits 32]
.stage1_farjump:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x00F00000
    jmp 0x7E00
    cli
    hlt
    jmp $

[bits 16]
%include 'boot/stage1/stage1_common.inc'
%include 'boot/stage1/stage1_gdt.inc'

section .data
boot_message_lgdt_panic:    db "Panic: Couldn't enable protected mode!", 0