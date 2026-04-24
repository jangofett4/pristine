/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <stddef.h>
#include <stdint.h>

#define PMM_HHDM_START   ((uintptr_t)0xFFFF800000000000ULL)

void      pmm_init(uint8_t *bitmap, const uint32_t size);
uint64_t  pmm_alloc(void);
void      pmm_free(uint64_t page);
void      pmm_set_bitmap_size(uint32_t bitmap_size);
size_t    pmm_get_free_page_count(void);