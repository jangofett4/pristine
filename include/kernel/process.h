/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <common/idt64.h>
#include <common/regs64.h>
#include <kernel/vmm.h>
#include <kernel/cpu.h>
#include <kernel/state.h>
#include <stdint.h>

// TODO: Currently userspace apps get a fixed PROCESS_USERSPACE_DEFAULT_STACK_SIZE bytes of stack
//       An on-demand paging can be implemented to support applications that require more stack

#define PROCESS_USERSPACE_DEFAULT_STACK_SIZE        0x200000    // 2 MiB
#define PROCESS_USERSPACE_DEFAULT_KERNEL_STACK_SIZE 0x4000      // 16 KiB

#define PROCESS_USERSPACE_VIRT_TOP              0x00007FFFFFFFF000ULL

//                                               v top                        v kernel stack top                            v guard
#define PROCESS_USERSPACE_VIRT_STACK_TOP        (PROCESS_USERSPACE_VIRT_TOP - PROCESS_USERSPACE_DEFAULT_KERNEL_STACK_SIZE - 0x1000)
#define PROCESS_USERSPACE_VIRT_KERNEL_STACK_TOP (PROCESS_USERSPACE_VIRT_TOP)

typedef enum {
    PROCESS_READY   = 0,
    PROCESS_RUNNING = 1,
    PROCESS_DEAD    = 2
} ProcessState;

typedef struct {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} ProcessContext;

struct Process {
    uint64_t      *pml4;
    uint32_t       pid;
    uintptr_t      entry;
    uintptr_t      stack_top;
    uintptr_t      kernel_stack_top;
    ProcessContext context;
    ProcessState   state;
};

typedef struct Process Process;

static inline void process_save_state(Process *process, const InterruptFrame *frame) {
    process->context.rax = frame->registers.rax;
    process->context.rbx = frame->registers.rbx;
    process->context.rcx = frame->registers.rcx;
    process->context.rdx = frame->registers.rdx;
    process->context.rsi = frame->registers.rsi;
    process->context.rdi = frame->registers.rdi;
    process->context.rbp = frame->registers.rbp;
    process->context.r8 = frame->registers.r8;
    process->context.r9 = frame->registers.r9;
    process->context.r10 = frame->registers.r10;
    process->context.r11 = frame->registers.r11;
    process->context.r12 = frame->registers.r12;
    process->context.r13 = frame->registers.r13;
    process->context.r14 = frame->registers.r14;
    process->context.r15 = frame->registers.r15;
    process->context.ss = frame->iret_frame.ss;
    process->context.rsp = frame->iret_frame.rsp;
    process->context.rflags = frame->iret_frame.rflags;
    process->context.cs = frame->iret_frame.cs;
    process->context.rip = frame->iret_frame.rip;
}

__attribute__((noreturn))
static inline void process_jump(const Process* process) {
    CpuState *cpu_state = get_cpu_state();
    cpu_state->tss->rsp0 = process->kernel_stack_top;
    vmm_switch(process->pml4);

    // If we are dropping privilege levels, swapgs
    if (process->context.cs & 0x03) {
        __asm__ volatile("swapgs");
    }

    __asm__ volatile (
        "mov %0, %%rsp\n"
        "pop %%rax\n"
        "pop %%rbx\n"
        "pop %%rcx\n"
        "pop %%rdx\n"
        "pop %%rsi\n"
        "pop %%rdi\n"
        "pop %%rbp\n"
        "pop %%r8\n"
        "pop %%r9\n"
        "pop %%r10\n"
        "pop %%r11\n"
        "pop %%r12\n"
        "pop %%r13\n"
        "pop %%r14\n"
        "pop %%r15\n"
        "iretq"
        : 
        : "r"(&process->context)
        : "memory"
    );

    __builtin_unreachable();
}