/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <stdarg.h>

// defines the maximum length of a formatted string
// for example printf_("str: %s", "abcdef") -> "str: abcdef", would use 6 formatter bytes
// another example printf_("hex: %08x", 0xFFFF) -> "num: 0000ffff", would use 8 formatter bytes
#define PRINTF_MAX_FORMAT_STACK     32

_Static_assert(PRINTF_MAX_FORMAT_STACK >= 19, "Format stack cannot be less than 19 bytes");

#define PRINTF_MAX_FORMATTER_LENGTH 16
#define PRINTF_MAX_WIDTH_MODIFIER   4
#define PRINTF_MAX_LENGTH_MODIFIER  2

extern void putchar_(const char c);

void printf_(const char *fmt, ...);
void vprintf_(const char *fmt, va_list list);