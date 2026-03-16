; Pristine
; stage1_memmap: memory map subroutines
; SPDX-License-Identifier: MIT

STAGE1_MEMMAP_ADDR  equ 0x6000 + 2
STAGE1_MEMMAP_SIZE  equ 20
STAGE1_MEMMAP_COUNT equ (STAGE1_MEMMAP_ADDR - 2)

stage1_memmap_get:
    push es
    push di
    mov ax, 0
    mov es, ax
    mov ax, STAGE1_MEMMAP_ADDR
    mov di, ax
    xor ebx, ebx
    mov word [STAGE1_MEMMAP_COUNT], 0
.loop:
    mov edx, 0x534D4150 ; 'SMAP'
    mov eax, 0xE820
    mov ecx, STAGE1_MEMMAP_SIZE
    int 0x15
    jc .error
    test ebx, ebx
    jz .exit
    add di, STAGE1_MEMMAP_SIZE
    inc word [STAGE1_MEMMAP_COUNT]
    jmp .loop
.error:
    jmp $
.exit:
    pop es
    pop di
    ret