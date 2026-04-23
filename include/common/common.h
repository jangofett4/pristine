/*
 * Pristine
 * common: common kernel definitions
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <kernel/kernel.h>
#include <kernel/pmm.h>

#include <stdint.h>

#define ALIGN_UP(value, align)   (((value) + (align) - 1) & ~((align) - 1))
#define ALIGN_DOWN(value, align) ((value) & ~((align) - 1))
#define DIV_CEIL(a, b)           (((a) + (b) - 1) / (b))

static inline void *phys_to_virt(uintptr_t phys) {
    return (void*)(uintptr_t)(phys + PMM_HHDM_START);
}

static inline uintptr_t virt_to_phys(void *virt) {
    return (uintptr_t)virt - PMM_HHDM_START;
}

static inline void *kernel_phys_to_virt(uintptr_t phys) {
    return (void*)(uintptr_t)(phys + KERNEL_BASE_OFFSET);
}

static inline uintptr_t kernel_virt_to_phys(void *virt) {
    return (uintptr_t)virt - KERNEL_BASE_OFFSET;
}

static inline int is_aligned(uint64_t addr, uint64_t alignment) {
    return (addr & (alignment -1)) == 0;
}