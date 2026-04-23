/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <kernel/process.h>
#include <common/idt64.h>

extern void scheduler_isr(void);

#define SCHEDULER_MAX_PROCESSES 32

bool scheduler_add_process(Process *process);
void scheduler_loop(void);