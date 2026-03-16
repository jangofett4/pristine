/*
 * Pristine
 * gdt: GDT definitions and functions
 * SPDX-License-Identifier MIT
 */

#include <common/gdt.h>
#include <stddef.h>
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

// this function will use index & index + 1 in the given gdt entry!
void gdt_set_tss_entry(uint64_t volatile *gdt, uint16_t index, void volatile *tss, uint8_t access, uint8_t flags, uint32_t count) {
    gdt[index] =
        ((uint64_t)count & 0xFFFF)      << 0  |
        ((uint64_t)tss & 0xFFFF)        << 16 |
        (((uint64_t)tss >> 16) & 0xFF)  << 32 |
        ((uint64_t)access)              << 40 |
        (((uint64_t)count >> 16) & 0xF) << 48 |
        ((uint64_t)flags)               << 52 |
        (((uint64_t)tss >> 24) & 0xFF)  << 56 ;
    gdt[index + 1] =
        (((uint64_t)tss >> 32) & 0xFFFFFFFF)  ;
}

void gdt_load_gdtr(uint64_t volatile *gdt, uint16_t count) {
    Gdt64Pointer ptr = {
        .base = (uintptr_t)gdt,
        .limit = (count * sizeof(uint64_t)) - 1
    };
    __asm__ volatile ("lgdt (%0)" : : "r"(&ptr) : "memory");
}

void gdt_load_tss(uint16_t selector) {
    __asm__ volatile ("ltr %0" : : "r"(selector));
}