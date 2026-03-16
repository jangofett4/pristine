/*
 * Pristine
 * idt: interrupt setup and handling functions
 * SPDX-License-Identifier: MIT
 */

#include <kernel/panic.h>
#include <common/idt64.h>
#include <common/pic.h>
#include <common/io.h>
#include <stdint.h>

#include <lib64/printf/printf.h>

// This is a bit big
static IDT64ISRDispatch dispatch_table[48] = {0};

void idt64_set_entry_int(IDT64Entry *idt_table, uint32_t index, uint32_t handler, uint8_t ist) {
    idt_table[index].offset1 = handler & 0xFFFF;
    idt_table[index].segment = 0x08;
    idt_table[index].ist.ist = ist;
    idt_table[index].ist.reserved = 0;
    idt_table[index].attr = IDT64_IDT_ATTR_INTERRUPT;
    idt_table[index].offset2 = (handler >> 16) & 0xFFFF;
}

void idt64_set_entry_trap(IDT64Entry *idt_table, uint32_t index, uint32_t handler, uint8_t ist) {
    idt_table[index].offset1 = handler & 0xFFFF;
    idt_table[index].segment = 0x08;
    idt_table[index].ist.ist = ist;
    idt_table[index].ist.reserved = 0;
    idt_table[index].attr = IDT64_IDT_ATTR_TRAP;
    idt_table[index].offset2 = (handler >> 16) & 0xFFFF;
}

void idt64_set_entries(IDT64Entry *entries, IDT64ISRHandler *handlers, size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (handlers[i].type == IDT64_ISR_INTERRUPT)
            idt64_set_entry_int(entries, i, (uint32_t)(uintptr_t)handlers[i].handler, handlers[i].ist);
        else
            idt64_set_entry_trap(entries, i, (uint32_t)(uintptr_t)handlers[i].handler, handlers[i].ist);
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
        printf("Error: Unhandled unmasked vector %d\n", frame->int_no);
        idt64_debug_print_frame(frame);
        if (frame->int_no < 32) {
            // CPU exception with no handler = panic, halt
            uint64_t cr2, cr3, fs, gs;
            __asm__ volatile("mov %%cr2, %0" : "=r"(cr2) :);
            __asm__ volatile("mov %%cr3, %0" : "=r"(cr3) :);
            __asm__ volatile("mov %%fs, %0" : "=r"(fs) :);
            __asm__ volatile("mov %%gs, %0" : "=r"(gs) :);
            KPANIC("Unhandled exception %d, error code 0x%x {\n CR2 = 0x%016x\n CR3 = 0x%016x\n FS = 0x%016x\n GS = 0x%016x\n}", frame->int_no, frame->err_code, cr2, cr3, fs, gs);
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
    printf("IDT64ISRFrame {\n");
    printf(" %-8s = 0x%016x\n", "rax", frame->rax);
    printf(" %-8s = 0x%016x\n", "rbx", frame->rbx);
    printf(" %-8s = 0x%016x\n", "rcx", frame->rcx);
    printf(" %-8s = 0x%016x\n", "rdx", frame->rdx);
    printf(" %-8s = 0x%016x\n", "rsi", frame->rsi);
    printf(" %-8s = 0x%016x\n", "rdi", frame->rdi);
    printf(" %-8s = 0x%016x\n", "rbp", frame->rbp);
    printf(" %-8s = 0x%016x\n", "r8", frame->r8);
    printf(" %-8s = 0x%016x\n", "r9", frame->r9);
    printf(" %-8s = 0x%016x\n", "r10", frame->r10);
    printf(" %-8s = 0x%016x\n", "r11", frame->r11);
    printf(" %-8s = 0x%016x\n", "r12", frame->r12);
    printf(" %-8s = 0x%016x\n", "r13", frame->r13);
    printf(" %-8s = 0x%016x\n", "r14", frame->r14);
    printf(" %-8s = 0x%016x\n", "r15", frame->r15);
    printf(" %-8s = 0x%016x\n", "int_no", frame->int_no);
    printf(" %-8s = 0x%016x\n", "err_code", frame->err_code);
    printf(" %-8s = 0x%016x\n", "rip", frame->rip);
    printf(" %-8s = 0x%016x\n", "cs", frame->cs);
    printf(" %-8s = 0x%016x\n", "rflags", frame->rflags);
    printf(" %-8s = 0x%016x\n", "ss", frame->ss);
    printf(" %-8s = 0x%016x\n", "rsp", frame->rsp);
    printf("}\n");
}