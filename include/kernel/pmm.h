/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <stdint.h>

#define PMM_HHDM_START   (0xFFFF800000000000ULL)

void      pmm_init(uint8_t *bitmap, const uint32_t size);
uint64_t  pmm_alloc(void);
void      pmm_free(uint64_t page);