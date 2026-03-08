/*
 * Pristine
 * stage2_serial - serial capabilities for stage 2 bootloader
 * SPDX-License-Identifier: MIT
 */
/*
 * File: stage2_serial.c
 * Project: Pristine
 * File Created: Saturday, 21st February 2026 7:41:50 pm
 * Author: Yahya Gedik (yahyagedikyg@gmail.com)
 * License: MIT
 */

#include "stage2_serial.h"
#include "stage2_io.h"

static Serial *__ptr_serial_default;

void serial_init(Serial *serial, uint16_t port) {
    serial->port = port;
    io_outb(port + 1, 0x00);    // disable interrupts
    io_outb(port + 3, 0x80);    // enable DLAB to set baud rate
    io_outb(port + 0, 0x03);    // baud divisor low byte (38400 baud)
    io_outb(port + 1, 0x00);    // baud divisor high byte
    io_outb(port + 3, 0x03);    // 8 bits, no parity, one stop bit, disable DLAB
    io_outb(port + 2, 0xC7);    // enable FIFO
    io_outb(port + 4, 0x0B);    // IRQs enabled, RTS/DSR set
}

void serial_putch(Serial *serial, char c) {
    while (!(io_inb(serial->port + 5) & 0x20)); 
    io_outb(serial->port, c);
}

void serial_puts(Serial *serial, const char *s) {
    while (*s) serial_putch(serial, *s++);
}

void serial_set_default(Serial *serial) {
    __ptr_serial_default = serial;
}

Serial* serial_get_default() {
    return __ptr_serial_default;
}