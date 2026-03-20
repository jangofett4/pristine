/*
 * Pristine
 * common: common kernel definitions
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <kernel/pmm.h>

#include <stdint.h>

#define ALIGN_UP(value, align)   (((value) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(value, align) ((value) & ~((align) - 1))

static inline void *phys_to_virt(uint64_t phys) {
    return (void*)(phys + PMM_HHDM_START);
}

static inline uint64_t virt_to_phys(void *virt) {
    return (uint64_t)virt - PMM_HHDM_START;
}