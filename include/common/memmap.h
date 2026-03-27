/*
 * Pristine
 * memmap: kernel memory map related subroutines
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <kernel/vmm.h>
#include <common/common.h>
#include <bitmap.h>
#include <stddef.h>
#include <stdint.h>

#define MEMMAP_MAX_ITEMS  48

typedef struct {
    uint32_t BaseAddrLow;
    uint32_t BaseAddrHigh;
    uint32_t LengthLow;
    uint32_t LengthHigh;
    uint32_t Type;
} __attribute__((packed)) MemmapEntry;

// This function walks the memory map entries and marks bitmap accordinly.
// Returns total usable memory in bytes
uint64_t memmap_bitmap_init(MemmapEntry *entries, size_t count, uint8_t *bitmap);