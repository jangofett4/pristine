/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

#define IDT64_SIZE 48

typedef struct {
    uint16_t offset1;   // bits 0-15 of handler address
    uint16_t segment;   // code segment selector (0x08)
    struct {
        uint8_t ist      : 4;
        uint8_t reserved : 4;
    } ist;
    uint8_t  attr;      // gate type, DPL, present bit
    uint16_t offset2;   // bits 16-31 of handler address
    uint32_t offset3;   // bits 32-63 of handler address
    uint32_t reserved;  // reserved
} __attribute__((packed)) IDT64Entry;

typedef struct {
    uint16_t limit;     // size of IDT - 1
    uint64_t base;      // address of IDT
} __attribute__((packed)) IDT64Ptr;

typedef struct {
    uint64_t rax;
    uint64_t rbx;
    uint64_t rcx;
    uint64_t rdx;
    uint64_t rsi;
    uint64_t rdi;
    uint64_t rbp;
    uint64_t r8;
    uint64_t r9;
    uint64_t r10;
    uint64_t r11;
    uint64_t r12;
    uint64_t r13;
    uint64_t r14;
    uint64_t r15;
    uint64_t int_no;
    uint64_t err_code;
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed)) IDT64ISRFrame;

typedef enum {
  IDT64_ISR_INTERRUPT,
  IDT64_ISR_TRAP  
} IDT64ISRHandlerType;

typedef struct {
    IDT64ISRHandlerType type;
    void (*handler)(void);
    uint8_t ist;
} IDT64ISRHandler;

typedef void (*IDT64ISRDispatch)(IDT64ISRFrame *frame);

#define IDT64_ISR_I(i, istidx) {.type=IDT64_ISR_INTERRUPT,.handler=idt64_isr_##i, .ist = istidx}
#define IDT64_ISR_T(i, istidx) {.type=IDT64_ISR_TRAP,.handler=idt64_isr_##i, .ist = istidx}

#define IDT64_IDT_ATTR_INTERRUPT  0x8E
#define IDT64_IDT_ATTR_TRAP       0x8F

static IDT64Entry __global_idt[IDT64_SIZE];

void idt64_init(void);
void idt64_load_idtr(IDT64Ptr *idt_ptr);
void idt64_set_entry_int(IDT64Entry *idt_table, uint32_t index, uint64_t handler, uint8_t ist);
void idt64_set_entry_trap(IDT64Entry *idt_table, uint32_t index, uint64_t handler, uint8_t ist);
void idt64_set_entries(IDT64Entry *entries, IDT64ISRHandler *handlers, size_t size);
void idt64_enable_interrupts(void);
void idt64_disable_interrupts(void);
void idt64_debug_print_frame(IDT64ISRFrame *frame);
void idt64_set_dispatch(uint8_t vector, IDT64ISRDispatch handler);

extern void idt64_isr_0(void);
extern void idt64_isr_1(void);
extern void idt64_isr_2(void);
extern void idt64_isr_3(void);
extern void idt64_isr_4(void);
extern void idt64_isr_5(void);
extern void idt64_isr_6(void);
extern void idt64_isr_7(void);
extern void idt64_isr_8(void);
extern void idt64_isr_9(void);
extern void idt64_isr_10(void);
extern void idt64_isr_11(void);
extern void idt64_isr_12(void);
extern void idt64_isr_13(void);
extern void idt64_isr_14(void);
extern void idt64_isr_15(void);
extern void idt64_isr_16(void);
extern void idt64_isr_17(void);
extern void idt64_isr_18(void);
extern void idt64_isr_19(void);
extern void idt64_isr_20(void);
extern void idt64_isr_21(void);
extern void idt64_isr_22(void);
extern void idt64_isr_23(void);
extern void idt64_isr_24(void);
extern void idt64_isr_25(void);
extern void idt64_isr_26(void);
extern void idt64_isr_27(void);
extern void idt64_isr_28(void);
extern void idt64_isr_29(void);
extern void idt64_isr_30(void);
extern void idt64_isr_31(void);