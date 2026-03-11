; Pristine
; stage1 - stage 1 bootloader, responsible for loading up stage 2 into memory, loading GDT, enabling 32 bit & jumping to stage 2 bootloader
; SPDX-License-Identifier: MIT

VESA_EXPECTED_BPP  equ 32

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

    call stage1_vesa_get_info
    cmp ax, 0x4f
    je .stage1_boot_vesa_vbe_found
    
    mov si, boot_message_vesa_panic
    call stage1_video_puts
    cli
    hlt
    jmp $

.stage1_boot_vesa_vbe_found:
    mov si, [VESA_VBE_INFO_BUF + VbeInfoBlock.VideoModePtr]

.stage1_boot_vesa_modes:
    mov ax, [VESA_VBE_INFO_BUF + VbeInfoBlock.VideoModePtr + 2]
    mov ds, ax
    mov cx, [ds:si]
    cmp cx, 0xffff
    je .stage1_boot_vesa_done_searching
    xor ax, ax
    mov ds, ax
    call stage1_vesa_get_mode_info
    add si, 2

    mov ax, [VESA_VBE_MODE_BUF + VbeModeInfoBlock.ModeAttributes]
    test ax, (1 << 0) ; is this mode available at all?
    jz .stage1_boot_vesa_modes
    test ax, (1 << 7) ; does it support linear framebuffer?
    jz .stage1_boot_vesa_modes

    ; check bpp
    mov al, [VESA_VBE_MODE_BUF + VbeModeInfoBlock.BitsPerPixel]
    cmp al, VESA_EXPECTED_BPP
    jne .stage1_boot_vesa_modes

    ; try 720p
    mov ax, [VESA_VBE_MODE_BUF + VbeModeInfoBlock.XResolution]
    cmp ax, 1280
    jne .stage1_boot_vesa_try_720p
    mov ax, [VESA_VBE_MODE_BUF + VbeModeInfoBlock.YResolution]
    cmp ax, 720
    jne .stage1_boot_vesa_try_720p
    mov [boot_vesa_1080p], cx
    jmp .stage1_boot_vesa_modes

.stage1_boot_vesa_try_720p:
    ; try 720p
    mov ax, [VESA_VBE_MODE_BUF + VbeModeInfoBlock.XResolution]
    cmp ax, 1024
    jne .stage1_boot_vesa_try_1024
    mov ax, [VESA_VBE_MODE_BUF + VbeModeInfoBlock.YResolution]
    cmp ax, 768
    jne .stage1_boot_vesa_try_1024
    mov [boot_vesa_720p], cx
    jmp .stage1_boot_vesa_modes

.stage1_boot_vesa_try_1024:
    ; try 1024x768
    mov ax, [VESA_VBE_MODE_BUF + VbeModeInfoBlock.XResolution]
    cmp ax, 800
    jne .stage1_boot_vesa_modes
    mov ax, [VESA_VBE_MODE_BUF + VbeModeInfoBlock.YResolution]
    cmp ax, 600
    jne .stage1_boot_vesa_modes
    mov [boot_vesa_1024], cx
    jmp .stage1_boot_vesa_modes

.stage1_boot_vesa_done_searching:
    mov cx, [boot_vesa_1080p]
    cmp cx, 0xffff
    jne .stage1_boot_vesa_setup
    mov cx, [boot_vesa_720p]
    cmp cx, 0xffff
    jne .stage1_boot_vesa_setup
    mov cx, [boot_vesa_1024]
    cmp cx, 0xffff
    jne .stage1_boot_vesa_setup
    jmp .stage1_boot_vesa_mode_not_found

.stage1_boot_vesa_mode_not_found:
    mov si, boot_message_vesa_mode_not_found
    call stage1_video_puts
    cli
    hlt
    jmp $

.stage1_boot_vesa_setup:
    call stage1_vesa_get_mode_info
    mov bx, cx
    or bx, 0x4000 ; linear framebuffer enable
    call stage1_vesa_switch_mode

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
%include 'boot/stage1/stage1_vesa.inc'

section .data
boot_vesa_1080p:                  dw 0xffff
boot_vesa_720p:                   dw 0xffff
boot_vesa_1024:                   dw 0xffff
boot_message_lgdt_panic:          db "1", 0
boot_message_vesa_panic:          db "2", 0
boot_message_vesa_mode_not_found: db "3", 0
