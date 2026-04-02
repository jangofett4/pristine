/*
 * Pristine
 * msr: MSR reading & writing functions
 * SPDX-License-Identifier MIT
 */

#include <kernel/msr.h>

uint64_t rdmsr(uint32_t msr) {
    //       eax  edx
    uint32_t low, high;
    __asm__ volatile(
        "rdmsr\n"
        : "=a"(low), "=d"(high)
        : "c"(msr)
    );
    return ((uint64_t)high << 32) | low;
}

void wrmsr(uint32_t msr, uint64_t value) {
    uint32_t low = value & 0xFFFFFFFF;
    uint32_t high = value >> 32;
    __asm__ volatile(
        "wrmsr\n"
        :
        : "c"(msr), "a"(low), "d"(high)
    );
}