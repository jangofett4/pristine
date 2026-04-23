/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <kernel/state.h>

static inline CpuState *get_cpu_state() {
    CpuState *state;
    __asm__ volatile(
        "mov %%gs:0, %0"
        : "=r"(state) 
    );
    return state;
}

extern GlobalState global_state;

static inline GlobalState *get_global_state() {
    return &global_state;
}