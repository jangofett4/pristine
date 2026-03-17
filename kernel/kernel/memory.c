/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#include "common/paging.h"
#include <kernel/panic.h>
#include <kernel/memory.h>
#include <stdint.h>

inline void kmem_bitmap_set(uint8_t *bitmap, const uint32_t index) {
    // if (kmem_bitmap_test(bitmap, index)) {
    //     KPANIC("kmem_bitmap_set: index %u is already set", index);
    // }
    const uint32_t slot = index / 8;
    bitmap[slot] |= 1 << (index % 8);
}

inline void kmem_bitmap_clear(uint8_t *bitmap, const uint32_t index) {
    // if (!kmem_bitmap_test(bitmap, index)) {
    //     KPANIC("kmem_bitmap_clear: index %u is already clear", index);
    // }
    const uint32_t slot = index / 8;
    bitmap[slot] &= ~(1 << (index % 8));
}

inline int kmem_bitmap_test(uint8_t *bitmap, const uint32_t index) {
    const uint32_t slot = index / 8;
    return bitmap[slot] & (1 << (index % 8));
}

uint32_t kmem_bitmap_find(uint8_t *bitmap, const uint32_t bit_count)
{
    const uint64_t *ptr = (const uint64_t*)bitmap;
    uint32_t word_count = bit_count / 64;
    for (uint32_t i = 0; i < word_count; i++) {
        uint64_t word = ptr[i];
        if (word != UINT64_MAX)
            return i * 64 + __builtin_ctzll(~word);
    }
    uint32_t remaining = bit_count % 64;
    if (remaining) {
        uint64_t word = ptr[word_count];
        uint64_t mask = (1ULL << remaining) - 1;
        word |= ~mask;
        if (word != UINT64_MAX)
            return word_count * 64 + __builtin_ctzll(~word);
    }
    return UINT32_MAX;
}

uint8_t *__pmm_bitmap;

// sets the default memory bitmap
void pmm_set_bitmap(uint8_t *bitmap) {
    __pmm_bitmap = bitmap;
}

// finds and returns a free page
void *pmm_alloc() {
    uint64_t idx = kmem_bitmap_find(__pmm_bitmap, MEMORY_BITMAP_SIZE * 8);
    if (idx == UINT32_MAX) {
        KPANIC("pmm_alloc: allocation failed, no memory left on device");
    }
    kmem_bitmap_set(__pmm_bitmap, idx);
    return (void*)(uintptr_t)(idx * PAGE_DEFAULT_SIZE);
}

void pmm_free(void* page) {
    uint32_t idx = ((uintptr_t)page / 4096);
    kmem_bitmap_clear(__pmm_bitmap, idx);
}