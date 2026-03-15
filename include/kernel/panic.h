/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <lib64/printf/printf.h>

#define KPANIC_SILENT() do { \
    __asm__ volatile("cli; hlt"); \
} while (0)

#define KPANIC(fmt, ...) do { \
    printf("KERNEL PANIC at %s:%d in %s(): " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    __asm__ volatile("cli; hlt"); \
} while(0)
