/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <stdint.h>

// values below give us 24 GiB or usable memory, should be plenty for our needs

#define MEMORY_HHDM_START   (0xFFFF800000000000ULL)

#define MEMORY_BITMAP_PHYS   0x100000
#define MEMORY_BITMAP_END   (MEMORY_BITMAP_PHYS + 0x0C0000)
#define MEMORY_BITMAP_SIZE  (MEMORY_BITMAP_END - MEMORY_BITMAP_PHYS)

void kmem_bitmap_set(uint8_t *bitmap, const uint32_t index);
void kmem_bitmap_clear(uint8_t *bitmap, const uint32_t index);

int kmem_bitmap_test(uint8_t *bitmap, const uint32_t index);

uint32_t kmem_bitmap_find(uint8_t *bitmap, const uint32_t bit_count);

void  pmm_set_bitmap(uint8_t *bitmap);
void *pmm_alloc(void);
void  pmm_free(void* address);