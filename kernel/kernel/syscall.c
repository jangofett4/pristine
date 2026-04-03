/*
 * Pristine
 * syscall: syscalls
 * SPDX-License-Identifier MIT
 */

#include <kernel/state.h>
#include <kernel/process.h>
#include <kernel/syscall.h>

static const GlobalState *global_state;

void syscall_init(const GlobalState *state) {
    global_state = state;
}

int64_t syscall_nop(uint64_t arg) {
    return SYSCALL_OK;
}

int64_t syscall_serial_puts(uint64_t arg) {
    if (arg == 0) return SYSCALL_ERR;
    Serial *serial = serial_get_default();
    if (serial) {
        // TODO: this is dangerous, user could have passed a kernel address for example and we would blindly print it
        serial_puts(serial, (const char*)(uintptr_t)arg);
        return SYSCALL_OK;
    }
    return SYSCALL_ERR;
}

int64_t syscall_exit(uint64_t arg) {
    const CpuState * state = get_cpu_state();
    state->current_process->state = PROCESS_DEAD;
    return SYSCALL_OK;
}

SyscallHandler syscall_table[] = {
    syscall_nop,
    syscall_serial_puts,
    syscall_exit,
};

// TODO: this is not optimal, requires memory access to check if syscall is in bounds
const uint64_t syscall_table_count = sizeof(syscall_table) / sizeof(syscall_table[0]);