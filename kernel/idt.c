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

static IDT64Callback isr_callbacks[IDT64_VECTOR_COUNT] = {0};

void idt64_init(IDT64Entry *idt_table) {
    memset(idt_table, 0, sizeof(IDT64Entry) * IDT64_VECTOR_COUNT);
    idt64_set_gate(idt_table,  0, (uint64_t)idt64_isr_0,  IDT64_ISR_TRAP,      0);
    idt64_set_gate(idt_table,  1, (uint64_t)idt64_isr_1,  IDT64_ISR_TRAP,      0);
    idt64_set_gate(idt_table,  2, (uint64_t)idt64_isr_2,  IDT64_ISR_INTERRUPT, 1);
    idt64_set_gate(idt_table,  3, (uint64_t)idt64_isr_3,  IDT64_ISR_TRAP,      0);
    idt64_set_gate(idt_table,  4, (uint64_t)idt64_isr_4,  IDT64_ISR_TRAP,      0);
    idt64_set_gate(idt_table,  5, (uint64_t)idt64_isr_5,  IDT64_ISR_TRAP,      0);
    idt64_set_gate(idt_table,  6, (uint64_t)idt64_isr_6,  IDT64_ISR_TRAP,      0);
    idt64_set_gate(idt_table,  7, (uint64_t)idt64_isr_7,  IDT64_ISR_TRAP,      0);
    idt64_set_gate(idt_table,  8, (uint64_t)idt64_isr_8,  IDT64_ISR_TRAP,      2);
    idt64_set_gate(idt_table,  9, (uint64_t)idt64_isr_9,  IDT64_ISR_TRAP,      0);
    idt64_set_gate(idt_table, 10, (uint64_t)idt64_isr_10, IDT64_ISR_TRAP,      0);
    idt64_set_gate(idt_table, 11, (uint64_t)idt64_isr_11, IDT64_ISR_TRAP,      0);
    idt64_set_gate(idt_table, 12, (uint64_t)idt64_isr_12, IDT64_ISR_TRAP,      0);
    idt64_set_gate(idt_table, 13, (uint64_t)idt64_isr_13, IDT64_ISR_TRAP,      0);
    idt64_set_gate(idt_table, 14, (uint64_t)idt64_isr_14, IDT64_ISR_TRAP,      0);
    idt64_set_gate(idt_table, 15, (uint64_t)idt64_isr_15, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 16, (uint64_t)idt64_isr_16, IDT64_ISR_TRAP,      0);
    idt64_set_gate(idt_table, 17, (uint64_t)idt64_isr_17, IDT64_ISR_TRAP,      0);
    idt64_set_gate(idt_table, 18, (uint64_t)idt64_isr_18, IDT64_ISR_TRAP,      3);
    idt64_set_gate(idt_table, 19, (uint64_t)idt64_isr_19, IDT64_ISR_TRAP,      0);
    idt64_set_gate(idt_table, 20, (uint64_t)idt64_isr_20, IDT64_ISR_TRAP,      0);
    idt64_set_gate(idt_table, 21, (uint64_t)idt64_isr_21, IDT64_ISR_TRAP,      0);
    idt64_set_gate(idt_table, 22, (uint64_t)idt64_isr_22, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 23, (uint64_t)idt64_isr_23, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 24, (uint64_t)idt64_isr_24, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 25, (uint64_t)idt64_isr_25, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 26, (uint64_t)idt64_isr_26, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 27, (uint64_t)idt64_isr_27, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 28, (uint64_t)idt64_isr_28, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 29, (uint64_t)idt64_isr_29, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 30, (uint64_t)idt64_isr_30, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 31, (uint64_t)idt64_isr_31, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 32, (uint64_t)pic_isr_32,   IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 33, (uint64_t)pic_isr_33,   IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 34, (uint64_t)pic_isr_34,   IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 35, (uint64_t)pic_isr_35,   IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 36, (uint64_t)pic_isr_36,   IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 37, (uint64_t)pic_isr_37,   IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 38, (uint64_t)pic_isr_38,   IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 39, (uint64_t)pic_isr_39,   IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 40, (uint64_t)pic_isr_40,   IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 41, (uint64_t)pic_isr_41,   IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 42, (uint64_t)pic_isr_42,   IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 43, (uint64_t)pic_isr_43,   IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 44, (uint64_t)pic_isr_44,   IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 45, (uint64_t)pic_isr_45,   IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 46, (uint64_t)pic_isr_46,   IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 47, (uint64_t)pic_isr_47,   IDT64_ISR_INTERRUPT, 0);
}

void idt64_load_idtr(IDT64Entry *idt_table, size_t count) {
    IDT64Descriptor idt_ptr = {
        .limit = (count * sizeof(IDT64Entry)) - 1,
        .base = (uint64_t)(uintptr_t)idt_table
    };
    
    __asm__ volatile(
        "lidt (%0)"
        :
        : "r"(&idt_ptr)
    );
}

void idt64_set_gate(IDT64Entry *idt_table, uint32_t index, uint64_t handler, IDT64GateType type, uint8_t ist) {
    idt_table[index].offset1 = handler & 0xFFFF;
    idt_table[index].segment = 0x08;
    idt_table[index].ist.index = ist;
    idt_table[index].ist.reserved = 0;
    idt_table[index].attr = type;
    idt_table[index].offset2 = (handler >> 16) & 0xFFFF;
    idt_table[index].offset3 = (handler >> 32) & 0xFFFFFFFF;
}

void idt64_enable_interrupts(void) {
    __asm__ volatile("sti");
}

void idt64_disable_interrupts(void) {
    __asm__ volatile("cli");
}

void idt64_set_callback(uint8_t vector, IDT64Callback callback) {
    isr_callbacks[vector] = callback;
}

void idt64_isr_dispatch(InterruptFrame *frame) {
    IDT64Callback handler = isr_callbacks[frame->int_no];
    
    if (!handler) {
        printf_("idt64_isr_dispatch: unhandled vector 0x%02lx\n", frame->int_no);
        idt64_debug_print_frame(frame);
        if (frame->int_no < 32) {
            // CPU exception with no handler = panic, halt
            uint64_t cr2, cr3, fs, gs;
            __asm__ volatile("mov %%cr2, %0" : "=r"(cr2) :);
            __asm__ volatile("mov %%cr3, %0" : "=r"(cr3) :);
            __asm__ volatile("mov %%fs, %0" : "=r"(fs) :);
            __asm__ volatile("mov %%gs, %0" : "=r"(gs) :);
            KPANIC("Unhandled exception 0x%02lx, error code 0x%02lx {\n CR2 = 0x%016lx\n CR3 = 0x%016lx\n FS = 0x%016lx\n GS = 0x%016lx\n}", frame->int_no, frame->err_code, cr2, cr3, fs, gs);
        }
    } else {
        handler();
    }

    // EOI regardless of whether we had a handler
    if (frame->int_no >= 40) {
        io_outb(PIC_SLAVE_CMD_PORT, PIC_EOI);
        io_outb(PIC_MASTER_CMD_PORT, PIC_EOI);
    }
    else if (frame->int_no >= 32) {
        io_outb(PIC_MASTER_CMD_PORT, PIC_EOI);
    }
}

void idt64_debug_print_frame(InterruptFrame *frame) {
    printf_("InterruptFrame {\n");
    printf_(" %-8s = 0x%016lx\n", "rax", frame->registers.rax);
    printf_(" %-8s = 0x%016lx\n", "rbx", frame->registers.rbx);
    printf_(" %-8s = 0x%016lx\n", "rcx", frame->registers.rcx);
    printf_(" %-8s = 0x%016lx\n", "rdx", frame->registers.rdx);
    printf_(" %-8s = 0x%016lx\n", "rsi", frame->registers.rsi);
    printf_(" %-8s = 0x%016lx\n", "rdi", frame->registers.rdi);
    printf_(" %-8s = 0x%016lx\n", "rbp", frame->registers.rbp);
    printf_(" %-8s = 0x%016lx\n", "r8", frame->registers.r8);
    printf_(" %-8s = 0x%016lx\n", "r9", frame->registers.r9);
    printf_(" %-8s = 0x%016lx\n", "r10", frame->registers.r10);
    printf_(" %-8s = 0x%016lx\n", "r11", frame->registers.r11);
    printf_(" %-8s = 0x%016lx\n", "r12", frame->registers.r12);
    printf_(" %-8s = 0x%016lx\n", "r13", frame->registers.r13);
    printf_(" %-8s = 0x%016lx\n", "r14", frame->registers.r14);
    printf_(" %-8s = 0x%016lx\n", "r15", frame->registers.r15);
    printf_(" %-8s = 0x%016lx\n", "int_no", frame->int_no);
    printf_(" %-8s = 0x%016lx\n", "err_code", frame->err_code);
    printf_(" %-8s = 0x%016lx\n", "rip", frame->iret_frame.rip);
    printf_(" %-8s = 0x%016lx\n", "cs", frame->iret_frame.cs);
    printf_(" %-8s = 0x%016lx\n", "rflags", frame->iret_frame.rflags);
    printf_(" %-8s = 0x%016lx\n", "ss", frame->iret_frame.ss);
    printf_(" %-8s = 0x%016lx\n", "rsp", frame->iret_frame.rsp);
    printf_("}\n");
}