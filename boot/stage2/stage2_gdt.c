/*
 * Pristine
 * stage2_gdt: GDT definitions and functions
 * SPDX-License-Identifier MIT
 */

#include <common/gdt.h>
#include <stdint.h>

void gdt_set_entry(uint64_t volatile *gdt, uint16_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags) {
    gdt[index] = 
        ((uint64_t)limit & 0xFFFF)      << 0  |
        ((uint64_t)base & 0xFFFF)       << 16 |
        (((uint64_t)base >> 16) & 0xFF) << 32 |
        ((uint64_t)access)              << 40 |
        (((uint64_t)limit >> 16) & 0xF) << 48 |
        ((uint64_t)flags)               << 52 |
        (((uint64_t)base >> 24) & 0xFF) << 56 ;
}

void gdt_load_gdtr(uint64_t volatile *gdt, uint16_t count) {
    Gdt32Pointer ptr = {
        .base = (uintptr_t)gdt,
        .limit = (count * sizeof(uint64_t)) - 1
    };
    __asm__ volatile ("lgdt (%0)" : : "r"(&ptr) : "memory");
}