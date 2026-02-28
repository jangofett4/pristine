; Pristine
; stage1_gdt - GDT definitions
; SPDX-License-Identifier: MIT


; Define a macro for GDT entries
%macro GDT_ENTRY 4
    dw (%2 & 0xFFFF)
    dw (%1 & 0xFFFF)
    db ((%1 >> 16) & 0xFF)
    db (%3)
    db ((%4 << 4) | ((%2 >> 16) & 0x0F))
    db ((%1 >> 24) & 0xFF)
%endmacro

stage1_gdt_start:
    GDT_ENTRY 0, 0, 0, 0            ; Null
    GDT_ENTRY 0, 0xFFFFF, 0x9A, 0xC ; Code (0x08)
    GDT_ENTRY 0, 0xFFFFF, 0x92, 0xC ; Data (0x10)
stage1_gdt_end:

stage1_gdt_ptr:
    dw stage1_gdt_end - stage1_gdt_start - 1
    dd stage1_gdt_start