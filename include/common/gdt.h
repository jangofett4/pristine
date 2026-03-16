/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <stdint.h>

#define GDT_ADDRESS     (0x7300)
#define GDT_MAX_ENTRIES (0x0100 / sizeof(uint64_t))

struct TSSEntry;

#define TSS_ADDRESS     (GDT_ADDRESS + GDT_MAX_ENTRIES)
#define TSS_MAX_ENTRIES (0x0100 / sizeof(TSSEntry))

typedef struct {
    uint32_t reserved0;
    // uint32_t rsp0_low;
    // uint32_t rsp0_high;
    // uint32_t rsp1_low;
    // uint32_t rsp1_high;
    // uint32_t rsp2_low;
    // uint32_t rsp2_high;
    uint64_t rsp0;
    uint64_t rsp1;
    uint64_t rsp2;
    uint32_t reserved1;
    uint32_t reserved2;
    uint64_t ist1;
    uint64_t ist2;
    uint64_t ist3;
    uint64_t ist4;
    uint64_t ist5;
    uint64_t ist6;
    uint64_t ist7;
    // uint32_t ist0_low;
    // uint32_t ist0_high;
    // uint32_t ist1_low;
    // uint32_t ist1_high;
    // uint32_t ist2_low;
    // uint32_t ist2_high;
    // uint32_t ist3_low;
    // uint32_t ist3_high;
    // uint32_t ist4_low;
    // uint32_t ist4_high;
    // uint32_t ist5_low;
    // uint32_t ist5_high;
    // uint32_t ist6_low;
    // uint32_t ist6_high;
    // uint32_t ist7_low;
    // uint32_t ist7_high;
    uint32_t reserved3;
    uint32_t reserved4;
    uint16_t reserved5;
    uint16_t iobp;
} __attribute__((packed)) TSSEntry;

typedef struct {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) Gdt32Pointer;

typedef struct {
    uint16_t limit;
    uint64_t base;
} __attribute__((packed)) Gdt64Pointer;

void gdt_set_entry(uint64_t volatile *gdt, uint16_t index, uint32_t base, uint32_t limit, uint8_t access, uint8_t flags);
void gdt_set_tss_entry(uint64_t volatile *gdt, uint16_t index, void volatile *tss, uint8_t access, uint8_t flags, uint32_t count);

void gdt_load_gdtr(uint64_t volatile *gdt, uint16_t count);
void gdt_load_tss(uint16_t selector);