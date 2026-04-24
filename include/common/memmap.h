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

typedef struct {
    uint64_t  memory_usable;
    uintptr_t memory_usable_top;
    uintptr_t memory_top;
} MemmapInitResult;

// This function walks the memory map entries and marks bitmap accordinly.
// Returns total usable memory in bytes
MemmapInitResult memmap_bitmap_init(MemmapEntry *entries, size_t count, uint8_t *bitmap);