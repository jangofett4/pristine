/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

uint8_t io_inb(uint16_t port);
uint16_t io_inw(uint16_t port);

void io_outb(uint16_t port, uint8_t data);
void io_outw(uint16_t port, uint16_t data);