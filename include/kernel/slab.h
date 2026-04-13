/*
 * Pristine
 * slab: Binning slab allocator declarations
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

#define SLAB_VIRT_START 0xFFFF900000000000ULL
#define SLAB_VIRT_SIZE  0x10000000000ULL // 1 TiB
#define SLAB_VIRT_END   (SLAB_VIRT_START + SLAB_VIRT_SIZE)

#define SLAB_TOTAL_BINS  24
#define SLAB_RECYCLE_MAX 256

typedef struct SlabHeader SlabHeader;

typedef struct SlabHeader {
    uint32_t    count_objects;
    void       *next_free;
    SlabHeader *next_slab;
    SlabHeader *prev_slab;
    uint64_t    phys;
    uint16_t    bin;
} SlabHeader;

_Static_assert(sizeof(SlabHeader) % 8 == 0, "sizeof(SlabHeader) needs to be a multiple of 8");

typedef struct {
    const uint32_t  object_size;
    const uint32_t  max_objects;
    SlabHeader     *free_slabs;
    SlabHeader     *full_slabs;
} BinHeader;

void *bin_alloc(uint16_t bin_idx);
bool  bin_free(void* addr);