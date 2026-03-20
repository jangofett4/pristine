/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <stdint.h>

#define KERNEL_BASE_PHYS      (0x400000)
#define KERNEL_END_PHYS       (KERNEL_BASE_PHYS + 0x200000)

#define KERNEL_BASE_ADDRESS   (0xFFFFFFFF80000000ULL)

extern uint8_t __kernel_ist1_start[];
extern uint8_t __kernel_rsp0_start[];
extern uint8_t __kernel_stack_start[];

static uint64_t __kernel_system_memory;