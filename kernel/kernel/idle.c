/*
 * Pristine
 * idle: kernel idle process
 * SPDX-License-Identifier: MIT
 */

#include <kernel/idle.h>
#include <kernel/syscall.h>

#include <printf.h>
#include <stddef.h>

void kernel_idle(void) {
    while (1) __asm__ volatile ("hlt");
}

void kernel_other_process1(void) {
    size_t i = 0;
    while (1) {
        printf_("Test from process 1\n");
        i++;
        if (i >= 10000) {
            syscall_exit(0);
        }
    }
    while (1) __asm__ volatile ("hlt");
}

void kernel_other_process2(void) {
    while (1) {
        printf_("Test from process 2\n");
    }
    while (1) __asm__ volatile ("hlt");
}