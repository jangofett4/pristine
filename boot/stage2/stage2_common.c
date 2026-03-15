/*
 * Pristine
 * stage2_common - common functions used throught the stage 2 bootloader
 * SPDX-License-Identifier: MIT
 */

#include <common/serial.h>

void _putchar(char ch) {
    Serial* serial = serial_get_default();
    if (serial)
        serial_putch(serial, ch);
}