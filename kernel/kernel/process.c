/*
 * Pristine
 * process: process management related functions & definitions
 * SPDX-License-Identifier MIT
 */

#include "kernel/vmm.h"
#include <kernel/process.h>

__attribute__((noreturn))
void process_start_trampoline(const Process *process) {
    vmm_switch(process->pml4);

    __asm__ volatile (
        "push $0x1B\n"
        "push %0\n"
        "push $0x200\n"
        "push $0x23\n"
        "push %1\n"
        "swapgs\n"
        "iretq\n"
        :
        : 
        "r"(process->stack_top),
        "r"(process->entry)
        :
    );

    __asm__ volatile ("cli; hlt;");
    __builtin_unreachable();
}