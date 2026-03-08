/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "stage2_kstdint.h"

#define SERIAL_COM1 0x3F8
#define SERIAL_COM2 0x2F8

typedef struct {
    uint16_t port;
} Serial;

void serial_init(Serial* serial, uint16_t port);
void serial_putch(Serial* serial, char data);
void serial_puts(Serial* serial, const char *data);
void serial_set_default(Serial *serial);
Serial* serial_get_default(void);