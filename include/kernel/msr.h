/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <stdint.h>

#define MSR_REG_EFER   0xC0000080

#define MSR_REG_STAR   0xC0000081
#define MSR_REG_LSTAR  0xC0000082
#define MSR_REG_CSTAR  0xC0000083
#define MSR_REG_SFMASK 0xC0000084

#define MSR_REG_FSBASE       0xC0000100
#define MSR_REG_GSBASE       0xC0000101
#define MSR_REG_KERNELGSBASE 0xC0000102

uint64_t rdmsr(uint32_t msr);
void wrmsr(uint32_t msr, uint64_t value);