/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <kernel/state.h>
#include <stdint.h>

#define SYSCALL_OK  0
#define SYSCALL_ERR -1

typedef int64_t (*SyscallHandler)(uint64_t arg);

extern SyscallHandler syscall_table[];

void syscall_init(const GlobalState *state);
extern void syscall_stub(void);