/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <stdint.h>

#define KERNEL_BASE_PHYS      (0x400000)
#define KERNEL_END_PHYS       (KERNEL_BASE_PHYS + 0x200000)

#define KERNEL_BASE_OFFSET    0xFFFFFFFF80000000ULL
#define KERNEL_BASE_VIRT      (KERNEL_BASE_OFFSET + KERNEL_BASE_PHYS)

extern uint8_t __kernel_ist1_top[];
extern uint8_t __kernel_ist2_top[];
extern uint8_t __kernel_ist3_top[];
extern uint8_t __kernel_rsp0_top[];
extern uint8_t __kernel_stack_top[];

extern uint8_t __kernel_bss_guard_base[];
extern uint8_t __kernel_stack_guard[];
extern uint8_t __kernel_rsp0_guard[];
extern uint8_t __kernel_ist3_guard[];
extern uint8_t __kernel_ist2_guard[];
extern uint8_t __kernel_ist1_guard[];

#define KERNEL_STACK_SIZE (0x1000 * 4)
#define KERNEL_RSP0_SIZE  (0x1000 * 1)
#define KERNEL_IST1_SIZE  (0x1000 * 1)
#define KERNEL_IST2_SIZE  (0x1000 * 1)
#define KERNEL_IST3_SIZE  (0x1000 * 1)

static inline void kernel_halt(void) {
    __asm__ volatile("hlt");
}