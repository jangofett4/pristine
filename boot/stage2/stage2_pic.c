/*
 * Pristine
 * stage2_pic - PIC initialization & functions
 * SPDX-License-Identifier: MIT
 */

#include "include/stage2_pic.h"
#include "include/stage2_io.h"
#include "include/stage2_idt.h"

void _pic_io_wait() {
    io_outb(0x80, 0x00);
}

void pic_init(void) {
    // ICW1
    io_outb(PIC_MASTER_CMD_PORT,  0x11); _pic_io_wait();
    io_outb(PIC_SLAVE_CMD_PORT,   0x11); _pic_io_wait();
    // ICW2
    io_outb(PIC_MASTER_DATA_PORT, 0x20); _pic_io_wait();  // IRQ0 = INT 32
    io_outb(PIC_SLAVE_DATA_PORT,  0x28); _pic_io_wait();  // IRQ8 = INT 40
    // ICW3
    io_outb(PIC_MASTER_DATA_PORT, 0x04); _pic_io_wait();  // master has slave on IRQ2
    io_outb(PIC_SLAVE_DATA_PORT,  0x02); _pic_io_wait();  // slave cascade identity
    // ICW4
    io_outb(PIC_MASTER_DATA_PORT, 0x01); _pic_io_wait();
    io_outb(PIC_SLAVE_DATA_PORT,  0x01); _pic_io_wait();
    // mask all IRQs for now
    io_outb(PIC_MASTER_DATA_PORT, 0xFF); _pic_io_wait();
    io_outb(PIC_SLAVE_DATA_PORT,  0xFF); _pic_io_wait();
}

void pic_isr_master_return() {
    io_outb(PIC_MASTER_CMD_PORT, 0x20);
}

void pic_isr_slave_return() {
    io_outb(PIC_SLAVE_CMD_PORT, 0x20);
    io_outb(PIC_MASTER_CMD_PORT, 0x20);
}

void pic_mask_irq(uint8_t irq) {
    uint16_t port = irq < 8 ? PIC_MASTER_DATA_PORT : PIC_SLAVE_DATA_PORT;
    uint8_t current_mask = io_inb(port);
    io_outb(port, current_mask | (1 << (irq % 8)));
}

void pic_unmask_irq(uint8_t irq) {
    uint16_t port = irq < 8 ? PIC_MASTER_DATA_PORT : PIC_SLAVE_DATA_PORT;
    uint8_t current_mask = io_inb(port);
    io_outb(port, current_mask & ~(1 << (irq % 8)));
}