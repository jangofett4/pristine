/*
 * Pristine
 * default: base implementations for several functions
 * SPDX-License-Identifier MIT
 */

#include <common/serial.h>

void _putchar(char ch) {
    Serial *serial = serial_get_default();
    if (serial) serial_putch(serial, ch);
}