/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <syscall.h>

#include <stdint.h>

static inline void exit(int status) {
    syscall1(SYS_EXIT, (uint64_t)status);
}

static inline void abort(void) {
    syscall1(SYS_EXIT, (uint64_t)-1);
}