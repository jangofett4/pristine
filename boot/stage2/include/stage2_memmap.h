/*
 * Pristine
 * stage2_memmap: memory map related definitions
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>

#define MEMMAP_ADDR       0x4000
#define MEMMAP_COUNT_ADDR (MEMMAP_ADDR-2)
#define MEMMAP_MAX_ITEMS  32

typedef struct {
    uint32_t BaseAddrLow;
    uint32_t BaseAddrHigh;
    uint32_t LengthLow;
    uint32_t LengthHigh;
    uint32_t Type;
} __attribute__((packed)) MemmapEntry;

uint16_t memmap_get_count(void);
MemmapEntry *memmap_get_storage(void);