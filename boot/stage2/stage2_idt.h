/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "stage2_kstdint.h"

#define IDT32_SIZE 48

typedef struct {
    uint16_t offset1;   // bits 0-15 of handler address
    uint16_t segment;   // code segment selector (0x08)
    uint8_t  zero;      // always 0
    uint8_t  attr;      // gate type, DPL, present bit
    uint16_t offset2;   // bits 16-31 of handler address
} __attribute__((packed)) IDT32Entry;

typedef struct {
    uint16_t limit;     // size of IDT - 1
    uint32_t base;      // address of IDT
} __attribute__((packed)) IDT32Ptr;

typedef struct {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp;
    uint32_t ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags;
} __attribute__((packed)) IDT32ISRFrame;

typedef enum {
  ISR_INTERRUPT,
  ISR_TRAP  
} IDT32ISRHandlerType;

typedef struct {
    IDT32ISRHandlerType type;
    void (*handler)(void);
} IDT32ISRHandler;

typedef void (*IDT32ISRDispatch)(IDT32ISRFrame *frame);

#define ISR_I(i) {.type=ISR_INTERRUPT,.handler=isr_##i}
#define ISR_T(i) {.type=ISR_TRAP,.handler=isr_##i}

#define IDT_ATTR_INTERRUPT  0x8E
#define IDT_ATTR_TRAP       0x8F

void idt32_load_idtr(IDT32Ptr *idt_ptr);
void idt32_set_entry_int(IDT32Entry *idt_table, uint32_t index, uint32_t handler);
void idt32_set_entry_trap(IDT32Entry *idt_table, uint32_t index, uint32_t handler);
void idt32_set_entries(IDT32Entry *entries, IDT32ISRHandler *handlers, size_t size);
void idt32_enable_interrupts(void);
void idt32_disable_interrupts(void);
void idt32_debug_print_frame(IDT32ISRFrame *frame);
void idt32_set_dispatch(uint8_t vector, IDT32ISRDispatch handler);

extern void isr_0(void);
extern void isr_1(void);
extern void isr_2(void);
extern void isr_3(void);
extern void isr_4(void);
extern void isr_5(void);
extern void isr_6(void);
extern void isr_7(void);
extern void isr_8(void);
extern void isr_9(void);
extern void isr_10(void);
extern void isr_11(void);
extern void isr_12(void);
extern void isr_13(void);
extern void isr_14(void);
extern void isr_15(void);
extern void isr_16(void);
extern void isr_17(void);
extern void isr_18(void);
extern void isr_19(void);
extern void isr_20(void);
extern void isr_21(void);
extern void isr_22(void);
extern void isr_23(void);
extern void isr_24(void);
extern void isr_25(void);
extern void isr_26(void);
extern void isr_27(void);
extern void isr_28(void);
extern void isr_29(void);
extern void isr_30(void);
extern void isr_31(void);