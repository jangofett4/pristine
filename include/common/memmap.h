/*
 * Pristine
 * memmap: kernel memory map related subroutines
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <stdint.h>

#define MEMMAP_MAX_ITEMS  48

typedef struct {
    uint32_t BaseAddrLow;
    uint32_t BaseAddrHigh;
    uint32_t LengthLow;
    uint32_t LengthHigh;
    uint32_t Type;
} __attribute__((packed)) MemmapEntry;