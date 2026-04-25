/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <syscall.h>

#include <stdint.h>

static inline void *sbrk(intptr_t count) {
    return (void*)syscall1(SYS_SBRK, count);
}