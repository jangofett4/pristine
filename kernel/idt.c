/*
 * Pristine
 * idt: interrupt setup and handling functions
 * SPDX-License-Identifier: MIT
 */

#include <kernel/panic.h>
#include <common/idt64.h>
#include <common/pic.h>
#include <common/io.h>
#include <common/string.h>
#include <stdint.h>

#include <printf.h>

// This is a bit big
static IDT64ISRDispatch dispatch_table[IDT64_SIZE] = {0};

void idt64_init(void) {
    memset(__global_idt, 0, sizeof(IDT64Entry) * IDT64_SIZE);
    idt64_set_entry_trap(__global_idt,  0, (uint64_t)idt64_isr_0,  1);
    idt64_set_entry_trap(__global_idt,  1, (uint64_t)idt64_isr_1,  1);
    idt64_set_entry_int (__global_idt,  2, (uint64_t)idt64_isr_2,  0);
    idt64_set_entry_trap(__global_idt,  3, (uint64_t)idt64_isr_3,  1);
    idt64_set_entry_trap(__global_idt,  4, (uint64_t)idt64_isr_4,  1);
    idt64_set_entry_trap(__global_idt,  5, (uint64_t)idt64_isr_5,  1);
    idt64_set_entry_trap(__global_idt,  6, (uint64_t)idt64_isr_6,  1);
    idt64_set_entry_trap(__global_idt,  7, (uint64_t)idt64_isr_7,  1);
    idt64_set_entry_trap(__global_idt,  8, (uint64_t)idt64_isr_8,  1);
    idt64_set_entry_trap(__global_idt,  9, (uint64_t)idt64_isr_9,  1);
    idt64_set_entry_trap(__global_idt, 10, (uint64_t)idt64_isr_10, 1);
    idt64_set_entry_trap(__global_idt, 11, (uint64_t)idt64_isr_11, 1);
    idt64_set_entry_trap(__global_idt, 12, (uint64_t)idt64_isr_12, 1);
    idt64_set_entry_trap(__global_idt, 13, (uint64_t)idt64_isr_13, 1);
    idt64_set_entry_trap(__global_idt, 14, (uint64_t)idt64_isr_14, 1);
    idt64_set_entry_int (__global_idt, 15, (uint64_t)idt64_isr_15, 0);
    idt64_set_entry_trap(__global_idt, 16, (uint64_t)idt64_isr_16, 1);
    idt64_set_entry_trap(__global_idt, 17, (uint64_t)idt64_isr_17, 1);
    idt64_set_entry_trap(__global_idt, 18, (uint64_t)idt64_isr_18, 1);
    idt64_set_entry_trap(__global_idt, 19, (uint64_t)idt64_isr_19, 1);
    idt64_set_entry_trap(__global_idt, 20, (uint64_t)idt64_isr_20, 1);
    idt64_set_entry_trap(__global_idt, 21, (uint64_t)idt64_isr_21, 1);
    idt64_set_entry_int (__global_idt, 22, (uint64_t)idt64_isr_22, 0);
    idt64_set_entry_int (__global_idt, 23, (uint64_t)idt64_isr_23, 0);
    idt64_set_entry_int (__global_idt, 24, (uint64_t)idt64_isr_24, 0);
    idt64_set_entry_int (__global_idt, 25, (uint64_t)idt64_isr_25, 0);
    idt64_set_entry_int (__global_idt, 26, (uint64_t)idt64_isr_26, 0);
    idt64_set_entry_int (__global_idt, 27, (uint64_t)idt64_isr_27, 0);
    idt64_set_entry_int (__global_idt, 28, (uint64_t)idt64_isr_28, 0);
    idt64_set_entry_int (__global_idt, 29, (uint64_t)idt64_isr_29, 0);
    idt64_set_entry_int (__global_idt, 30, (uint64_t)idt64_isr_30, 0);
    idt64_set_entry_int (__global_idt, 31, (uint64_t)idt64_isr_31, 0);
    idt64_set_entry_int (__global_idt, 32, (uint64_t)pic_isr_32,   0);
    idt64_set_entry_int (__global_idt, 33, (uint64_t)pic_isr_33,   0);
    idt64_set_entry_int (__global_idt, 34, (uint64_t)pic_isr_34,   0);
    idt64_set_entry_int (__global_idt, 35, (uint64_t)pic_isr_35,   0);
    idt64_set_entry_int (__global_idt, 36, (uint64_t)pic_isr_36,   0);
    idt64_set_entry_int (__global_idt, 37, (uint64_t)pic_isr_37,   0);
    idt64_set_entry_int (__global_idt, 38, (uint64_t)pic_isr_38,   0);
    idt64_set_entry_int (__global_idt, 39, (uint64_t)pic_isr_39,   0);
    idt64_set_entry_int (__global_idt, 40, (uint64_t)pic_isr_40,   0);
    idt64_set_entry_int (__global_idt, 41, (uint64_t)pic_isr_41,   0);
    idt64_set_entry_int (__global_idt, 42, (uint64_t)pic_isr_42,   0);
    idt64_set_entry_int (__global_idt, 43, (uint64_t)pic_isr_43,   0);
    idt64_set_entry_int (__global_idt, 44, (uint64_t)pic_isr_44,   0);
    idt64_set_entry_int (__global_idt, 45, (uint64_t)pic_isr_45,   0);
    idt64_set_entry_int (__global_idt, 46, (uint64_t)pic_isr_46,   0);
    idt64_set_entry_int (__global_idt, 47, (uint64_t)pic_isr_47,   0);
}

void idt64_set_entry_int(IDT64Entry *idt_table, uint32_t index, uint64_t handler, uint8_t ist) {
    idt_table[index].offset1 = handler & 0xFFFF;
    idt_table[index].segment = 0x08;
    idt_table[index].ist.ist = ist;
    idt_table[index].ist.reserved = 0;
    idt_table[index].attr = IDT64_IDT_ATTR_INTERRUPT;
    idt_table[index].offset2 = (handler >> 16) & 0xFFFF;
    idt_table[index].offset3 = (handler >> 32) & 0xFFFFFFFF;
}

void idt64_set_entry_trap(IDT64Entry *idt_table, uint32_t index, uint64_t handler, uint8_t ist) {
    idt_table[index].offset1 = handler & 0xFFFF;
    idt_table[index].segment = 0x08;
    idt_table[index].ist.ist = ist;
    idt_table[index].ist.reserved = 0;
    idt_table[index].attr = IDT64_IDT_ATTR_TRAP;
    idt_table[index].offset2 = (handler >> 16) & 0xFFFF;
    idt_table[index].offset3 = (handler >> 32) & 0xFFFFFFFF;
}

void idt64_set_entries(IDT64Entry *entries, IDT64ISRHandler *handlers, size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (handlers[i].type == IDT64_ISR_INTERRUPT)
            idt64_set_entry_int(entries, i, (uint64_t)(uintptr_t)handlers[i].handler, handlers[i].ist);
        else
            idt64_set_entry_trap(entries, i, (uint64_t)(uintptr_t)handlers[i].handler, handlers[i].ist);
    }
}

void idt64_enable_interrupts(void) {
    __asm__ volatile("sti");
}

void idt64_disable_interrupts(void) {
    __asm__ volatile("cli");
}

void idt64_set_dispatch(uint8_t vector, IDT64ISRDispatch handler) {
    dispatch_table[vector] = handler;
}

void idt64_isr_handler(IDT64ISRFrame *frame) {
    IDT64ISRDispatch handler = dispatch_table[frame->int_no];
    
    if (!handler) {
        // printf("idt64_isr_handler: unhandled vector 0x%016x\n", frame->int_no);
        // idt64_debug_print_frame(frame);
        if (frame->int_no < 32) {
            // CPU exception with no handler = panic, halt
            uint64_t cr2, cr3, fs, gs;
            __asm__ volatile("mov %%cr2, %0" : "=r"(cr2) :);
            __asm__ volatile("mov %%cr3, %0" : "=r"(cr3) :);
            __asm__ volatile("mov %%fs, %0" : "=r"(fs) :);
            __asm__ volatile("mov %%gs, %0" : "=r"(gs) :);
            KPANIC("Unhandled exception 0x%lx, error code 0x%lx {\n CR2 = 0x%016lx\n CR3 = 0x%016lx\n FS = 0x%016lx\n GS = 0x%016lx\n}", frame->int_no, frame->err_code, cr2, cr3, fs, gs);
        }
    } else {
        handler(frame);
    }

    // EOI regardless of whether we had a handler
    if (frame->int_no >= 40) {
        io_outb(PIC_SLAVE_CMD_PORT, 0x20);
        io_outb(PIC_MASTER_CMD_PORT, 0x20);
    }
    else if (frame->int_no >= 32) {
        io_outb(PIC_MASTER_CMD_PORT, 0x20);
    }
}

void idt64_debug_print_frame(IDT64ISRFrame *frame) {
    printf_("IDT64ISRFrame {\n");
    printf_(" %-8s = 0x%016lx\n", "rax", frame->rax);
    printf_(" %-8s = 0x%016lx\n", "rbx", frame->rbx);
    printf_(" %-8s = 0x%016lx\n", "rcx", frame->rcx);
    printf_(" %-8s = 0x%016lx\n", "rdx", frame->rdx);
    printf_(" %-8s = 0x%016lx\n", "rsi", frame->rsi);
    printf_(" %-8s = 0x%016lx\n", "rdi", frame->rdi);
    printf_(" %-8s = 0x%016lx\n", "rbp", frame->rbp);
    printf_(" %-8s = 0x%016lx\n", "r8", frame->r8);
    printf_(" %-8s = 0x%016lx\n", "r9", frame->r9);
    printf_(" %-8s = 0x%016lx\n", "r10", frame->r10);
    printf_(" %-8s = 0x%016lx\n", "r11", frame->r11);
    printf_(" %-8s = 0x%016lx\n", "r12", frame->r12);
    printf_(" %-8s = 0x%016lx\n", "r13", frame->r13);
    printf_(" %-8s = 0x%016lx\n", "r14", frame->r14);
    printf_(" %-8s = 0x%016lx\n", "r15", frame->r15);
    printf_(" %-8s = 0x%016lx\n", "int_no", frame->int_no);
    printf_(" %-8s = 0x%016lx\n", "err_code", frame->err_code);
    printf_(" %-8s = 0x%016lx\n", "rip", frame->rip);
    printf_(" %-8s = 0x%016lx\n", "cs", frame->cs);
    printf_(" %-8s = 0x%016lx\n", "rflags", frame->rflags);
    printf_(" %-8s = 0x%016lx\n", "ss", frame->ss);
    printf_(" %-8s = 0x%016lx\n", "rsp", frame->rsp);
    printf_("}\n");
}