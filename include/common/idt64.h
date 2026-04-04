/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <common/regs64.h>

#include <stdint.h>
#include <stddef.h>

#define IDT64_VECTOR_COUNT 256

#define IDT64_ATTR_INTERRUPT  0x8E
#define IDT64_ATTR_TRAP       0x8F

typedef struct {
    uint16_t offset1;   // bits 0-15 of handler address
    uint16_t segment;   // code segment selector (0x08)
    struct {
        uint8_t index    : 4;
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
} __attribute__((packed)) IDT64Descriptor;

typedef struct {
    uint64_t rip;
    uint64_t cs;
    uint64_t rflags;
    uint64_t rsp;
    uint64_t ss;
} __attribute__((packed)) IretFrame;

typedef struct {
    Registers64 registers;  // Saved by software stub
    uint64_t    int_no;     // Pushed by stub
    uint64_t    err_code;   // Pushed by CPU or stub (dummy)
    IretFrame   iret_frame; // Pushed by CPU
} __attribute__((packed)) InterruptFrame;

typedef enum {
  IDT64_ISR_INTERRUPT,
  IDT64_ISR_TRAP  
} IDT64GateType;

typedef struct {
    IDT64GateType type;
    void (*handler)(void);
    uint8_t ist;
} IDT64EntryConfig;

typedef void (*IDT64Callback)(void);

void idt64_init(IDT64Entry *idt_table);
void idt64_load_idtr(IDT64Entry *entries, size_t count) ;
void idt64_set_gate(IDT64Entry *idt_table, uint32_t index, uint64_t handler, IDT64GateType type, uint8_t ist);
void idt64_enable_interrupts(void);
void idt64_disable_interrupts(void);
void idt64_debug_print_frame(InterruptFrame *frame);
void idt64_set_callback(uint8_t vector, IDT64Callback callback);

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