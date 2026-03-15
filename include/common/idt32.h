/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

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
  IDT32_ISR_INTERRUPT,
  IDT32_ISR_TRAP  
} IDT32ISRHandlerType;

typedef struct {
    IDT32ISRHandlerType type;
    void (*handler)(void);
} IDT32ISRHandler;

typedef void (*IDT32ISRDispatch)(IDT32ISRFrame *frame);

#define IDT32_ISR_I(i) {.type=IDT32_ISR_INTERRUPT,.handler=idt32_isr_##i}
#define IDT32_ISR_T(i) {.type=IDT32_ISR_TRAP,.handler=idt32_isr_##i}

#define IDT32_IDT_ATTR_INTERRUPT  0x8E
#define IDT32_IDT_ATTR_TRAP       0x8F

void idt32_load_idtr(IDT32Ptr *idt_ptr);
void idt32_set_entry_int(IDT32Entry *idt_table, uint32_t index, uint32_t handler);
void idt32_set_entry_trap(IDT32Entry *idt_table, uint32_t index, uint32_t handler);
void idt32_set_entries(IDT32Entry *entries, IDT32ISRHandler *handlers, size_t size);
void idt32_enable_interrupts(void);
void idt32_disable_interrupts(void);
void idt32_debug_print_frame(IDT32ISRFrame *frame);
void idt32_set_dispatch(uint8_t vector, IDT32ISRDispatch handler);

extern void idt32_isr_0(void);
extern void idt32_isr_1(void);
extern void idt32_isr_2(void);
extern void idt32_isr_3(void);
extern void idt32_isr_4(void);
extern void idt32_isr_5(void);
extern void idt32_isr_6(void);
extern void idt32_isr_7(void);
extern void idt32_isr_8(void);
extern void idt32_isr_9(void);
extern void idt32_isr_10(void);
extern void idt32_isr_11(void);
extern void idt32_isr_12(void);
extern void idt32_isr_13(void);
extern void idt32_isr_14(void);
extern void idt32_isr_15(void);
extern void idt32_isr_16(void);
extern void idt32_isr_17(void);
extern void idt32_isr_18(void);
extern void idt32_isr_19(void);
extern void idt32_isr_20(void);
extern void idt32_isr_21(void);
extern void idt32_isr_22(void);
extern void idt32_isr_23(void);
extern void idt32_isr_24(void);
extern void idt32_isr_25(void);
extern void idt32_isr_26(void);
extern void idt32_isr_27(void);
extern void idt32_isr_28(void);
extern void idt32_isr_29(void);
extern void idt32_isr_30(void);
extern void idt32_isr_31(void);