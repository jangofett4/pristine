/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "stage2_kstdint.h"

#define PIC_MASTER_CMD_PORT 0x20
#define PIC_SLAVE_CMD_PORT  0xA0

#define PIC_MASTER_DATA_PORT    0x21
#define PIC_SLAVE_DATA_PORT     0xA1

void pic_init(void);
void pic_isr_master_return();
void pic_isr_slave_return();
void pic_mask_irq(uint8_t irq);
void pic_unmask_irq(uint8_t irq);

extern void isr_32(void);
extern void isr_33(void);
extern void isr_34(void);
extern void isr_35(void);
extern void isr_36(void);
extern void isr_37(void);
extern void isr_38(void);
extern void isr_39(void);
extern void isr_40(void);
extern void isr_41(void);
extern void isr_42(void);
extern void isr_43(void);
extern void isr_44(void);
extern void isr_45(void);
extern void isr_46(void);
extern void isr_47(void);