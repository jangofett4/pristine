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

void pic_init(void);
void pic_mask_irq(uint8_t irq);
void pic_unmask_irq(uint8_t irq);

extern void pic_isr_32(void);
extern void pic_isr_33(void);
extern void pic_isr_34(void);
extern void pic_isr_35(void);
extern void pic_isr_36(void);
extern void pic_isr_37(void);
extern void pic_isr_38(void);
extern void pic_isr_39(void);
extern void pic_isr_40(void);
extern void pic_isr_41(void);
extern void pic_isr_42(void);
extern void pic_isr_43(void);
extern void pic_isr_44(void);
extern void pic_isr_45(void);
extern void pic_isr_46(void);
extern void pic_isr_47(void);