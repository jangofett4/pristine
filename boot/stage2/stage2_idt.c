/*
 * Pristine
 * stage2_idt - interrupt setup and handling functions
 * SPDX-License-Identifier: MIT
 */

#include "stage2_idt.h"
#include "stage2_common.h"
#include "stage2_pic.h"
#include "printf.h"

// This is a bit big
static isr_dispatch_t dispatch_table[48] = {0};

void idt32_set_entry_int(idt32_entry_t *idt_table, uint32_t index, uint32_t handler) {
    idt_table[index].offset1 = handler & 0xFFFF;
    idt_table[index].segment = 0x08;
    idt_table[index].zero = 0;
    idt_table[index].attr = IDT_ATTR_INTERRUPT;
    idt_table[index].offset2 = (handler >> 16) & 0xFFFF;
}

void idt32_set_entry_trap(idt32_entry_t *idt_table, uint32_t index, uint32_t handler) {
    idt_table[index].offset1 = handler & 0xFFFF;
    idt_table[index].segment = 0x08;
    idt_table[index].zero = 0;
    idt_table[index].attr = IDT_ATTR_TRAP;
    idt_table[index].offset2 = (handler >> 16) & 0xFFFF;
}

void idt32_set_entries(idt32_entry_t *entries, idt32_isr_handler_t *handlers, size_t size) {
    for (size_t i = 0; i < size; i++) {
        if (handlers[i].type == ISR_INTERRUPT)
            idt32_set_entry_int(entries, i, (uint32_t)handlers[i].handler);
        else
            idt32_set_entry_trap(entries, i, (uint32_t)handlers[i].handler);
    }
}

void idt32_enable_interrupts(void) {
    __asm__ volatile("sti");
}

void idt32_disable_interrupts(void) {
    __asm__ volatile("cli");
}

void idt32_set_dispatch(uint8_t vector, isr_dispatch_t handler) {
    dispatch_table[vector] = handler;
}

void idt32_isr_handler(idt32_isr_frame_t *frame) {
    isr_dispatch_t handler = dispatch_table[frame->int_no];
    
    if (!handler) {
        printf("Error: Unhandled unmasked vector %d\n", frame->int_no);
        idt32_debug_print_frame(frame);
        if (frame->int_no < 32) {
            // CPU exception with no handler = panic, halt
            PANIC("Unhandled exception %d, error code 0x%x", frame->int_no, frame->err_code);
        }
    } else {
        handler(frame);
    }

    // EOI regardless of whether we had a handler
    if (frame->int_no >= 40)
        pic_isr_slave_return();
    else if (frame->int_no >= 32)
        pic_isr_master_return();
}

void idt32_debug_print_frame(idt32_isr_frame_t *frame) {
    printf(nameof(idt32_isr_frame_t) " {\n");
    printf(" %-8s = 0x%08x\n", "gs", frame->gs); 
    printf(" %-8s = 0x%08x\n", "fs", frame->fs); 
    printf(" %-8s = 0x%08x\n", "es", frame->es); 
    printf(" %-8s = 0x%08x\n", "ds", frame->ds); 
    printf(" %-8s = 0x%08x\n", "edi", frame->edi); 
    printf(" %-8s = 0x%08x\n", "esi", frame->esi); 
    printf(" %-8s = 0x%08x\n", "ebp", frame->ebp); 
    printf(" %-8s = 0x%08x\n", "esp", frame->esp); 
    printf(" %-8s = 0x%08x\n", "ebx", frame->ebx); 
    printf(" %-8s = 0x%08x\n", "edx", frame->edx); 
    printf(" %-8s = 0x%08x\n", "ecx", frame->ecx); 
    printf(" %-8s = 0x%08x\n", "eax", frame->eax); 
    printf(" %-8s = 0x%08x\n", "int_no", frame->int_no); 
    printf(" %-8s = 0x%08x\n", "err_code", frame->err_code); 
    printf(" %-8s = 0x%08x\n", "eip", frame->eip); 
    printf(" %-8s = 0x%08x\n", "cs", frame->cs); 
    printf(" %-8s = 0x%08x\n", "eflags", frame->eflags); 
    printf("}\n");
}