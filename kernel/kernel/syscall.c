/*
 * Pristine
 * syscall: syscalls
 * SPDX-License-Identifier MIT
 */

#include <kernel/state.h>
#include <kernel/process.h>
#include <kernel/syscall.h>
#include <kernel/scheduler.h>

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
    const CpuState *state = get_cpu_state();
    state->current_process->state = PROCESS_DEAD;
    state->current_process->exit_code = (int)arg;
    scheduler_yield();
    __builtin_unreachable();
}

int64_t syscall_yield(uint64_t arg) {
    scheduler_yield();
    __builtin_unreachable();
}

int64_t syscall_sbrk(uint64_t arg) {
    const CpuState *state = get_cpu_state();
    const int64_t request = (int64_t)arg;
    if (request < 0) {
        if (state->current_process->brk + request < state->current_process->brk_start) {
            return 0;
        }
        const uintptr_t old_page = ALIGN_UP(state->current_process->brk, VMM_DEFAULT_PAGE_SIZE) / VMM_DEFAULT_PAGE_SIZE;
        const uintptr_t new_page = ALIGN_UP(state->current_process->brk + request, VMM_DEFAULT_PAGE_SIZE) / VMM_DEFAULT_PAGE_SIZE;
        for (uintptr_t page = new_page; page < old_page; page++) {
            vmm_free(state->current_process->pml4, (void*)(page * VMM_DEFAULT_PAGE_SIZE));
        }
        const uintptr_t ret = state->current_process->brk;
        state->current_process->brk += request;
        return ret;
    } else {
        const uintptr_t start       = ALIGN_UP(state->current_process->brk, VMM_DEFAULT_PAGE_SIZE);
        const uintptr_t end         = ALIGN_UP(state->current_process->brk + request, VMM_DEFAULT_PAGE_SIZE);
        const size_t required_pages = (end / VMM_DEFAULT_PAGE_SIZE) - (start / VMM_DEFAULT_PAGE_SIZE);
        for (size_t i = 0; i < required_pages; i++) {
            const uintptr_t phys = pmm_alloc();
            vmm_map(state->current_process->pml4, phys, start + i * VMM_DEFAULT_PAGE_SIZE, VMM_FLAGS_USER_DATA);
        }
        const uintptr_t ret = state->current_process->brk;
        state->current_process->brk += request;
        return ret;
    }
}

SyscallHandler syscall_table[] = {
    syscall_exit,
    syscall_sbrk, 
    syscall_yield,
    syscall_serial_puts,
};

// TODO: this is not optimal, requires memory access to check if syscall is in bounds
const uint64_t syscall_table_count = sizeof(syscall_table) / sizeof(syscall_table[0]);