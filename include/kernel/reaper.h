/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <kernel/state.h>

bool reaper_add_process(Process *process);
void reaper_run(void);