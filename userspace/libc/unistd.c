/*
 * Pristine
 * unistd: TODO: explain what unistd is
 * SPDX-License-Identifier: MIT
 */

#include <unistd.h>

void *sbrk(intptr_t count) {
    return (void*)syscall1(SYS_SBRK, count);
}