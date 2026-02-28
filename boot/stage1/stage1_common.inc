; Pristine
; stage1_common - "commonly" used subroutines for stage 1 bootloader
; SPDX-License-Identifier: MIT

stage1_video_clear:
    mov ah, 0x06
    mov al, 0x00
    mov bh, 0x0F
    mov ch, 0x00
    mov cl, 0x00
    mov dh, 0xff
    mov dl, 0xff
    int 0x10
    ret

stage1_reset_cursor:
    mov ah, 0x02
    mov bh, 0x00
    mov dh, 0x00
    mov dl, 0x00
    int 0x10
    ret

stage1_video_putch:
    mov ah, 0x0A
    mov bh, 0x00
    int 0x10

stage1_video_puts:
    mov ah, 0x0E
    mov bh, 0x00
.loop:
    mov al, [si]
    cmp al, 0x00
    jz .exit
    int 0x10
    inc si
    jmp .loop
.exit:
    ret

;; Disk routines

stage1_readdisk:    
    mov ah, 0x42
    mov dl, [boot_drive]
    mov si, _asm_dap
    int 0x13
    jc .disk_error
    ret
.disk_error:
    mov si, boot_message_diskerr
    call stage1_video_puts
    jmp $

section .data
boot_drive:             db 0
boot_message_diskerr:   db "Disk read error!", 0
_asm_dap:
    db 0x10         ; DAP size
    db 0x00         ; reserved
    dw 63           ; sectors to read
    dw 0x7E00       ; buffer offset
    dw 0x0000       ; buffer segment
    dq 1            ; LBA start (sector 1, right after stage 1)