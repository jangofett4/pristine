/*
 * Pristine
 * scheduler: round robin preemptive process scheduler
 * SPDX-License-Identifier: MIT
 */

#include <printf.h>
#include <kernel/process.h>
#include <common/idt64.h>
#include <kernel/lapic.h>
#include <kernel/state.h>
#include <kernel/scheduler.h>

Process *process_list[SCHEDULER_MAX_PROCESSES];

bool scheduler_add_process(Process *process) {
    for (size_t i = 0; i < SCHEDULER_MAX_PROCESSES; i++) {
        if (process_list[i] == NULL) {
            process_list[i] = process;
            return true;
        }
    }
    return false;
}

__attribute__((noreturn))
void scheduler_loop(void) {
    CpuState *cpu_state = get_cpu_state();
    const InterruptFrame *frame = cpu_state->interrupt_frame;
    lapic_eoi(cpu_state->lapic);

    Process* process_to_continue = NULL;

    size_t loops = 0;
    for (; loops < SCHEDULER_MAX_PROCESSES ; loops++, cpu_state->scheduler_next++) {
        if (cpu_state->scheduler_next >= SCHEDULER_MAX_PROCESSES)
            cpu_state->scheduler_next = 0;

        if (process_list[cpu_state->scheduler_next] != NULL) {
            process_to_continue = process_list[cpu_state->scheduler_next];
            break;
        }
    }
    cpu_state->scheduler_next++;

    if (process_to_continue == NULL) {
        process_to_continue = cpu_state->idle_process;
    } 

    if (process_to_continue != cpu_state->current_process) {
        process_save_state(cpu_state->current_process, frame);
        cpu_state->current_process = process_to_continue;
    }
    process_jump(process_to_continue);
}