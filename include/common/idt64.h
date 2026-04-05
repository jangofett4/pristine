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
extern void idt64_isr_32(void);
extern void idt64_isr_33(void);
extern void idt64_isr_34(void);
extern void idt64_isr_35(void);
extern void idt64_isr_36(void);
extern void idt64_isr_37(void);
extern void idt64_isr_38(void);
extern void idt64_isr_39(void);
extern void idt64_isr_40(void);
extern void idt64_isr_41(void);
extern void idt64_isr_42(void);
extern void idt64_isr_43(void);
extern void idt64_isr_44(void);
extern void idt64_isr_45(void);
extern void idt64_isr_46(void);
extern void idt64_isr_47(void);
extern void idt64_isr_48(void);
extern void idt64_isr_49(void);
extern void idt64_isr_50(void);
extern void idt64_isr_51(void);
extern void idt64_isr_52(void);
extern void idt64_isr_53(void);
extern void idt64_isr_54(void);
extern void idt64_isr_55(void);
extern void idt64_isr_56(void);
extern void idt64_isr_57(void);
extern void idt64_isr_58(void);
extern void idt64_isr_59(void);
extern void idt64_isr_60(void);
extern void idt64_isr_61(void);
extern void idt64_isr_62(void);
extern void idt64_isr_63(void);
extern void idt64_isr_64(void);
extern void idt64_isr_65(void);
extern void idt64_isr_66(void);
extern void idt64_isr_67(void);
extern void idt64_isr_68(void);
extern void idt64_isr_69(void);
extern void idt64_isr_70(void);
extern void idt64_isr_71(void);
extern void idt64_isr_72(void);
extern void idt64_isr_73(void);
extern void idt64_isr_74(void);
extern void idt64_isr_75(void);
extern void idt64_isr_76(void);
extern void idt64_isr_77(void);
extern void idt64_isr_78(void);
extern void idt64_isr_79(void);
extern void idt64_isr_80(void);
extern void idt64_isr_81(void);
extern void idt64_isr_82(void);
extern void idt64_isr_83(void);
extern void idt64_isr_84(void);
extern void idt64_isr_85(void);
extern void idt64_isr_86(void);
extern void idt64_isr_87(void);
extern void idt64_isr_88(void);
extern void idt64_isr_89(void);
extern void idt64_isr_90(void);
extern void idt64_isr_91(void);
extern void idt64_isr_92(void);
extern void idt64_isr_93(void);
extern void idt64_isr_94(void);
extern void idt64_isr_95(void);
extern void idt64_isr_96(void);
extern void idt64_isr_97(void);
extern void idt64_isr_98(void);
extern void idt64_isr_99(void);
extern void idt64_isr_100(void);
extern void idt64_isr_101(void);
extern void idt64_isr_102(void);
extern void idt64_isr_103(void);
extern void idt64_isr_104(void);
extern void idt64_isr_105(void);
extern void idt64_isr_106(void);
extern void idt64_isr_107(void);
extern void idt64_isr_108(void);
extern void idt64_isr_109(void);
extern void idt64_isr_110(void);
extern void idt64_isr_111(void);
extern void idt64_isr_112(void);
extern void idt64_isr_113(void);
extern void idt64_isr_114(void);
extern void idt64_isr_115(void);
extern void idt64_isr_116(void);
extern void idt64_isr_117(void);
extern void idt64_isr_118(void);
extern void idt64_isr_119(void);
extern void idt64_isr_120(void);
extern void idt64_isr_121(void);
extern void idt64_isr_122(void);
extern void idt64_isr_123(void);
extern void idt64_isr_124(void);
extern void idt64_isr_125(void);
extern void idt64_isr_126(void);
extern void idt64_isr_127(void);
extern void idt64_isr_128(void);
extern void idt64_isr_129(void);
extern void idt64_isr_130(void);
extern void idt64_isr_131(void);
extern void idt64_isr_132(void);
extern void idt64_isr_133(void);
extern void idt64_isr_134(void);
extern void idt64_isr_135(void);
extern void idt64_isr_136(void);
extern void idt64_isr_137(void);
extern void idt64_isr_138(void);
extern void idt64_isr_139(void);
extern void idt64_isr_140(void);
extern void idt64_isr_141(void);
extern void idt64_isr_142(void);
extern void idt64_isr_143(void);
extern void idt64_isr_144(void);
extern void idt64_isr_145(void);
extern void idt64_isr_146(void);
extern void idt64_isr_147(void);
extern void idt64_isr_148(void);
extern void idt64_isr_149(void);
extern void idt64_isr_150(void);
extern void idt64_isr_151(void);
extern void idt64_isr_152(void);
extern void idt64_isr_153(void);
extern void idt64_isr_154(void);
extern void idt64_isr_155(void);
extern void idt64_isr_156(void);
extern void idt64_isr_157(void);
extern void idt64_isr_158(void);
extern void idt64_isr_159(void);
extern void idt64_isr_160(void);
extern void idt64_isr_161(void);
extern void idt64_isr_162(void);
extern void idt64_isr_163(void);
extern void idt64_isr_164(void);
extern void idt64_isr_165(void);
extern void idt64_isr_166(void);
extern void idt64_isr_167(void);
extern void idt64_isr_168(void);
extern void idt64_isr_169(void);
extern void idt64_isr_170(void);
extern void idt64_isr_171(void);
extern void idt64_isr_172(void);
extern void idt64_isr_173(void);
extern void idt64_isr_174(void);
extern void idt64_isr_175(void);
extern void idt64_isr_176(void);
extern void idt64_isr_177(void);
extern void idt64_isr_178(void);
extern void idt64_isr_179(void);
extern void idt64_isr_180(void);
extern void idt64_isr_181(void);
extern void idt64_isr_182(void);
extern void idt64_isr_183(void);
extern void idt64_isr_184(void);
extern void idt64_isr_185(void);
extern void idt64_isr_186(void);
extern void idt64_isr_187(void);
extern void idt64_isr_188(void);
extern void idt64_isr_189(void);
extern void idt64_isr_190(void);
extern void idt64_isr_191(void);
extern void idt64_isr_192(void);
extern void idt64_isr_193(void);
extern void idt64_isr_194(void);
extern void idt64_isr_195(void);
extern void idt64_isr_196(void);
extern void idt64_isr_197(void);
extern void idt64_isr_198(void);
extern void idt64_isr_199(void);
extern void idt64_isr_200(void);
extern void idt64_isr_201(void);
extern void idt64_isr_202(void);
extern void idt64_isr_203(void);
extern void idt64_isr_204(void);
extern void idt64_isr_205(void);
extern void idt64_isr_206(void);
extern void idt64_isr_207(void);
extern void idt64_isr_208(void);
extern void idt64_isr_209(void);
extern void idt64_isr_210(void);
extern void idt64_isr_211(void);
extern void idt64_isr_212(void);
extern void idt64_isr_213(void);
extern void idt64_isr_214(void);
extern void idt64_isr_215(void);
extern void idt64_isr_216(void);
extern void idt64_isr_217(void);
extern void idt64_isr_218(void);
extern void idt64_isr_219(void);
extern void idt64_isr_220(void);
extern void idt64_isr_221(void);
extern void idt64_isr_222(void);
extern void idt64_isr_223(void);
extern void idt64_isr_224(void);
extern void idt64_isr_225(void);
extern void idt64_isr_226(void);
extern void idt64_isr_227(void);
extern void idt64_isr_228(void);
extern void idt64_isr_229(void);
extern void idt64_isr_230(void);
extern void idt64_isr_231(void);
extern void idt64_isr_232(void);
extern void idt64_isr_233(void);
extern void idt64_isr_234(void);
extern void idt64_isr_235(void);
extern void idt64_isr_236(void);
extern void idt64_isr_237(void);
extern void idt64_isr_238(void);
extern void idt64_isr_239(void);
extern void idt64_isr_240(void);
extern void idt64_isr_241(void);
extern void idt64_isr_242(void);
extern void idt64_isr_243(void);
extern void idt64_isr_244(void);
extern void idt64_isr_245(void);
extern void idt64_isr_246(void);
extern void idt64_isr_247(void);
extern void idt64_isr_248(void);
extern void idt64_isr_249(void);
extern void idt64_isr_250(void);
extern void idt64_isr_251(void);
extern void idt64_isr_252(void);
extern void idt64_isr_253(void);
extern void idt64_isr_254(void);
extern void idt64_isr_255(void);