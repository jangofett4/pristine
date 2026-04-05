/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>

#define PIC_MASTER_CMD_PORT 0x20
#define PIC_SLAVE_CMD_PORT  0xA0

#define PIC_MASTER_DATA_PORT    0x21
#define PIC_SLAVE_DATA_PORT     0xA1

#define PIC_EOI 0x20

void pic_init(void);
void pic_mask_irq(uint8_t irq);
void pic_mask_all(void);
void pic_unmask_irq(uint8_t irq);