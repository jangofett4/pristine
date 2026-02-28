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
} serial_t;

void serial_init(serial_t* serial, uint16_t port);
void serial_putch(serial_t* serial, char data);
void serial_puts(serial_t* serial, const char *data);
void serial_set_default(serial_t *serial);
serial_t* serial_get_default(void);