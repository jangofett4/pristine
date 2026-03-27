/*
 * Pristine
 * panic: kernel panic functions
 * SPDX-License-Identifier MIT
 */

#include <kernel/panic.h>
#include <printf.h>

__attribute__((noreturn, used)) void kpanic(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vprintf_(fmt, args);
    va_end(args);
    __asm__ volatile ("cli; hlt;");
    __builtin_unreachable();
}

__attribute__((noreturn, used)) void kpanic_silent(void) {
    __asm__ volatile ("cli; hlt;");
    __builtin_unreachable();
}