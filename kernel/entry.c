/*
 * Pristine
 * entry - kernel entry point
 * SPDX-License-Identifier: MIT
 */

#include <stdint.h>

void _putchar(char ch) {}

void kmain(void) {
    uint8_t *mbr = (uint8_t*)0x7DFE;
    uint8_t mbr0 = mbr[1];
    uint8_t mbr1 = mbr[2];

    uint8_t *valid = (uint8_t*)(0x200000 * 7);
    valid[0] = 255;

    while(1);
}