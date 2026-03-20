/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <stdint.h>

struct TSSEntry;

typedef struct {
    uint32_t reserved0;
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

void gdt_reload_cs(uint16_t cs);
void gdt_reload_segments(uint16_t ds);