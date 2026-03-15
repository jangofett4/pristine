/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

uint8_t io_inb(const uint16_t port);
uint16_t io_inw(const uint16_t port);

void io_outb(const uint16_t port, uint8_t data);
void io_outw(const uint16_t port, uint16_t data);