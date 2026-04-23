/*
 * Pristine
 * idle: kernel idle process
 * SPDX-License-Identifier: MIT
 */

#include <printf.h>
#include <kernel/idle.h>

void kernel_idle(void) {
    while (1) __asm__ volatile ("hlt");
}

void kernel_other_process1(void) {
    while (1) {
        printf_("Test from process 1\n");
    }
    while (1) __asm__ volatile ("hlt");
}

void kernel_other_process2(void) {
    while (1) {
        printf_("Test from process 2\n");
    }
    while (1) __asm__ volatile ("hlt");
}