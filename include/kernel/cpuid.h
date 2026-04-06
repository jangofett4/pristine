/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

typedef struct {
    uint32_t eax, ebx, ecx, edx;
} Cpuid;

static inline Cpuid cpuid_subleaf(uint32_t eax, uint32_t ecx) {
    Cpuid result;
    __asm__ volatile (
        "cpuid"
        : "=a"(result.eax), "=b"(result.ebx), "=c"(result.ecx), "=d"(result.edx)
        : "a"(eax), "c"(ecx)
        : 
    );
    return result;
}

static inline Cpuid cpuid(uint32_t eax) {
    return cpuid_subleaf(eax, 0);
}