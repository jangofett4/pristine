/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#include <kernel/panic.h>
#include <kernel/vmm.h>
#include <kernel/pmm.h>
#include <common/string.h>
#include <bitmap.h>

#include <stdint.h>

uint32_t __pmm_bitmap_size;
uint8_t *__pmm_bitmap;

// sets the default memory bitmap
void pmm_init(uint8_t *bitmap, const uint32_t size) {
    __pmm_bitmap = bitmap;
    __pmm_bitmap_size = size;
}

// finds and returns a free page address
uint64_t pmm_alloc(void) {
    uint32_t idx = bitmap_find(__pmm_bitmap, __pmm_bitmap_size * 8);

    if (idx == UINT32_MAX) {
        KPANIC("pmm_alloc: allocation failed, no memory left on device");
    }

    #ifdef PRISTINE_DEBUG
    if (bitmap_test(__pmm_bitmap, idx)) {
        KPANIC("pmm_alloc: bitmap_find returned already set index");
    }
    #endif

    bitmap_set(__pmm_bitmap, idx);
    return (uint64_t)idx * VMM_DEFAULT_PAGE_SIZE;
}

// frees given page
void pmm_free(uint64_t page) {
    #ifdef PRISTINE_DEBUG
    if (page & (VMM_DEFAULT_PAGE_SIZE - 1))
        KPANIC("pmm_free: unaligned address %lx", page);
    #endif
    uint32_t idx = page / VMM_DEFAULT_PAGE_SIZE;
    #ifdef PRISTINE_DEBUG
    if (!bitmap_test(__pmm_bitmap, idx))
        KPANIC("pmm_free: double free at %lx", page);
    #endif
    
    bitmap_clear(__pmm_bitmap, idx);
}

void pmm_set_bitmap_size(uint32_t bitmap_size) {
    __pmm_bitmap_size = bitmap_size;
}

// this function walks the whole bitmap and counts empty pages.
// this is a rather expensive operation
size_t pmm_get_free_page_count(void) {
    size_t count = 0;
    for (size_t i = 0; i < __pmm_bitmap_size * 8; i++) {
        if (bitmap_test(__pmm_bitmap, i)) {
            count++;
        }
    }
    return count;
}