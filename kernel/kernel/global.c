/*
 * Pristine
 * default: base implementations for several functions
 * SPDX-License-Identifier MIT
 */

#include <kernel/kernel.h>
#include <kernel/global.h>

void _putchar(char ch) {
    Serial *serial = serial_get_default();
    if (serial) serial_putch(serial, ch);
}