/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <kernel/cpu.h>
#include <kernel/lapic.h>
#include <kernel/state.h>
#include <kernel/process.h>
#include <common/idt64.h>

extern void scheduler_isr(void);

#define SCHEDULER_MAX_PROCESSES 32

bool scheduler_add_process(Process *process);
void scheduler_loop(void);

static inline void scheduler_yield(void) {
    const CpuState *cpu_state = get_cpu_state();
    lapic_timer_set_counter(cpu_state->lapic, cpu_state->lapic_timer_speed);
    __asm__ volatile("int $0xFF");
}