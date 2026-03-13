/*
 * Pristine
 * stage2_memmap: memory map related definitions
 * SPDX-License-Identifier: MIT
 */

#include <common/memmap.h>
#include <stdint.h>

static MemmapEntry _memmap[MEMMAP_MAX_ITEMS];

uint16_t memmap_get_count(void) {
    return *(uint16_t*)MEMMAP_COUNT_ADDR;
}

MemmapEntry *memmap_get_storage(void) {
    return _memmap;
}