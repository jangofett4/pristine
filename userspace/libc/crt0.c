/*
 * Pristine
 * crt0: C Runtime 0 file to be linked against all userspace apps
 * SPDX-License-Identifier MIT
 */

extern int main();

void _start() {
    main();

    __asm__ volatile (
        "syscall"
        :
        : "a"(60), "D"(0)
        : "rcx", "r11"
    );

    __builtin_unreachable();
}