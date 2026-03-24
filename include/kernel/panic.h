/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <stdarg.h>
#include <lib64/printf/printf.h>
#include <stdarg.h>

#define KPANIC_SILENT() kpanic_silent();

#define KPANIC(fmt, ...) kpanic("KERNEL PANIC at %s:%d in %s(): " fmt "\n", __FILE__, __LINE__, __func__, ##__VA_ARGS__)

__attribute__((noreturn, used)) void kpanic(const char* fmt, ...);
__attribute__((noreturn, used)) void kpanic_silent(void);
