/*
 * Pristine
 * pit: Programmable Interval Timer functions
 * SPDX-License-Identifier: MIT
 */

// PIT will be used to calibrate APIC timer
// For this reason, PIT will be kept simple

#include <common/io.h>
#include <printf.h>
#include <kernel/pit.h>

void pit_init(uint8_t channel, uint16_t hz) {
    if (channel > 2) return;
    io_outb(PIT_PORT_COMMAND, (channel << 6) | 0b00110110);
    channel = channel == 0 ? PIT_PORT_CHANNEL0 : channel == 1 ? PIT_PORT_CHANNEL1 : PIT_PORT_CHANNEL2;
    const uint16_t final_divisor = PIT_CLOCK_RATE / hz;
    io_outb(channel, final_divisor & 0xFF);
    io_outb(channel, final_divisor >> 8);
}