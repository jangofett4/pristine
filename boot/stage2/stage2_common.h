/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include "lib32/printf/printf.h"

void uint32_to_str(char buf[10], uint32_t value);

#define nameof(x) #x
#define PANIC(fmt, ...) do { \
    printf("PANIC at %s:%d in %s(): " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    __asm__ volatile ("cli; hlt"); \
} while(0)

#define ASSERT(x, fmt, ...) do {\
    if (!x) {\
        PANIC(fmt, ##__VA_ARGS__);\
    }\
} while (0)