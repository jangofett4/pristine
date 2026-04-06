/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <common/regs64.h>
#include <common/idt64.h>

#define PIT_CLOCK_RATE    1193180

#define PIT_PORT_CHANNEL0 0x40
#define PIT_PORT_CHANNEL1 0x41
#define PIT_PORT_CHANNEL2 0x42

#define PIT_PORT_COMMAND  0x43

void pit_init(uint8_t channel, uint16_t hz);