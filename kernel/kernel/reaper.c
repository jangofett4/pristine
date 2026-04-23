/*
 * Pristine
 * reaper: incremental process reaper
 * SPDX-License-Identifier: MIT
 */

#include <kernel/reaper.h>
#include <kernel/scheduler.h>
#include <kernel/process.h>
#include <kernel/kmalloc.h>
#include <common/idt64.h>

static Process *process_list_reap[SCHEDULER_MAX_PROCESSES];
static size_t   reaper_cursor = 0;

bool reaper_add_process(Process *process) {
    for (size_t i = 0; i < SCHEDULER_MAX_PROCESSES; i++) {
        if (process_list_reap[i] == NULL) {
            process_list_reap[i] = process;
            return true;
        }
    }
    return false;
}

void reaper_run(void) {
    while (1) {
        if (reaper_cursor >= SCHEDULER_MAX_PROCESSES) reaper_cursor = 0;

        idt64_disable_interrupts();
        Process *process_to_reap = process_list_reap[reaper_cursor];
        process_list_reap[reaper_cursor] = NULL;
        idt64_enable_interrupts();

        if (process_to_reap != NULL) {
            process_destroy(process_to_reap);
            kfree(process_to_reap);
        }

        reaper_cursor++;

        scheduler_loop();
    }
}