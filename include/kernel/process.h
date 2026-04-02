/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <stdint.h>

// TODO: Currently userspace apps get a fixed PROCESS_USERSPACE_DEFAULT_STACK_SIZE bytes of stack
//       An on-demand paging can be implemented to support applications that require more stack

#define PROCESS_USERSPACE_DEFAULT_STACK_SIZE        0x200000    // 2 MiB
#define PROCESS_USERSPACE_DEFAULT_KERNEL_STACK_SIZE 0x4000      // 16 KiB

#define PROCESS_USERSPACE_VIRT_TOP              0x00007FFFFFFFF000ULL

//                                               v top                        v kernel stack top                            v guard
#define PROCESS_USERSPACE_VIRT_STACK_TOP        (PROCESS_USERSPACE_VIRT_TOP - PROCESS_USERSPACE_DEFAULT_KERNEL_STACK_SIZE - 0x1000)
#define PROCESS_USERSPACE_VIRT_KERNEL_STACK_TOP (PROCESS_USERSPACE_VIRT_TOP)

typedef struct {
    uint64_t *pml4;
    uint64_t  entry;
    uint64_t  stack_top;
    uint64_t  kernel_stack_top;
} Process;

void process_start_trampoline(const Process *process);