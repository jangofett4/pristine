/*
 * Pristine
 * process: process management related functions & definitions
 * SPDX-License-Identifier: MIT
 */

#include <common/common.h>
#include <common/string.h>
#include <kernel/pmm.h>
#include <kernel/vmm.h>
#include <kernel/process.h>

bool process_create(Process *process, uint32_t pid, uintptr_t entry, uint64_t cs, uint64_t ss, uint64_t rflags, uintptr_t stack_top, size_t stack_size, uintptr_t kernel_stack_top, size_t kernel_stack_size, uint64_t* kernel_pml4) {
    uintptr_t  pml4_phys = pmm_alloc();
    uint64_t  *pml4      = phys_to_virt(pml4_phys);
    memset(pml4, 0, VMM_DEFAULT_PAGE_SIZE);

    process->pml4      = pml4;
    process->pml4_phys = pml4_phys;
    process->pid       = pid;

    for (size_t s = stack_top - stack_size; s < stack_top; s += VMM_DEFAULT_PAGE_SIZE) {
        uintptr_t stack_phys = pmm_alloc();
        vmm_map(pml4, stack_phys, s, VMM_FLAGS_USER_DATA);
    }

    for (size_t s = kernel_stack_top - kernel_stack_size; s < kernel_stack_top; s += VMM_DEFAULT_PAGE_SIZE) {
        uintptr_t stack_phys = pmm_alloc();
        vmm_map(pml4, stack_phys, s, VMM_FLAGS_KERNEL_DATA);
    }

    process->stack_size = stack_size;
    process->kernel_stack_size = kernel_stack_size;

    process->context.cs = cs;
    process->context.ss = ss;
    process->context.rip = entry;
    process->context.rsp = stack_top;
    process->context.rflags = rflags;

    process->entry = entry;
    process->stack_top = stack_top;
    process->kernel_stack_top = kernel_stack_top;
    process->state = PROCESS_READY;

    return true;
}