/*
 * Pristine
 * crt0: C Runtime 0 file to be linked against all userspace apps
 * SPDX-License-Identifier MIT
 */

#include <stdlib.h>

extern int main();

void _start() {
    main();

    exit(0);
    
    while(1) __asm__ volatile("pause");
    __builtin_unreachable();
}