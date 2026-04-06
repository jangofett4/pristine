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
    idt64_set_gate(idt_table, 32, (uint64_t)idt64_isr_32, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 33, (uint64_t)idt64_isr_33, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 34, (uint64_t)idt64_isr_34, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 35, (uint64_t)idt64_isr_35, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 36, (uint64_t)idt64_isr_36, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 37, (uint64_t)idt64_isr_37, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 38, (uint64_t)idt64_isr_38, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 39, (uint64_t)idt64_isr_39, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 40, (uint64_t)idt64_isr_40, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 41, (uint64_t)idt64_isr_41, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 42, (uint64_t)idt64_isr_42, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 43, (uint64_t)idt64_isr_43, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 44, (uint64_t)idt64_isr_44, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 45, (uint64_t)idt64_isr_45, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 46, (uint64_t)idt64_isr_46, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 47, (uint64_t)idt64_isr_47, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 48, (uint64_t)idt64_isr_48, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 49, (uint64_t)idt64_isr_49, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 50, (uint64_t)idt64_isr_50, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 51, (uint64_t)idt64_isr_51, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 52, (uint64_t)idt64_isr_52, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 53, (uint64_t)idt64_isr_53, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 54, (uint64_t)idt64_isr_54, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 55, (uint64_t)idt64_isr_55, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 56, (uint64_t)idt64_isr_56, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 57, (uint64_t)idt64_isr_57, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 58, (uint64_t)idt64_isr_58, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 59, (uint64_t)idt64_isr_59, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 60, (uint64_t)idt64_isr_60, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 61, (uint64_t)idt64_isr_61, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 62, (uint64_t)idt64_isr_62, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 63, (uint64_t)idt64_isr_63, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 64, (uint64_t)idt64_isr_64, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 65, (uint64_t)idt64_isr_65, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 66, (uint64_t)idt64_isr_66, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 67, (uint64_t)idt64_isr_67, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 68, (uint64_t)idt64_isr_68, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 69, (uint64_t)idt64_isr_69, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 70, (uint64_t)idt64_isr_70, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 71, (uint64_t)idt64_isr_71, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 72, (uint64_t)idt64_isr_72, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 73, (uint64_t)idt64_isr_73, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 74, (uint64_t)idt64_isr_74, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 75, (uint64_t)idt64_isr_75, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 76, (uint64_t)idt64_isr_76, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 77, (uint64_t)idt64_isr_77, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 78, (uint64_t)idt64_isr_78, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 79, (uint64_t)idt64_isr_79, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 80, (uint64_t)idt64_isr_80, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 81, (uint64_t)idt64_isr_81, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 82, (uint64_t)idt64_isr_82, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 83, (uint64_t)idt64_isr_83, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 84, (uint64_t)idt64_isr_84, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 85, (uint64_t)idt64_isr_85, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 86, (uint64_t)idt64_isr_86, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 87, (uint64_t)idt64_isr_87, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 88, (uint64_t)idt64_isr_88, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 89, (uint64_t)idt64_isr_89, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 90, (uint64_t)idt64_isr_90, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 91, (uint64_t)idt64_isr_91, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 92, (uint64_t)idt64_isr_92, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 93, (uint64_t)idt64_isr_93, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 94, (uint64_t)idt64_isr_94, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 95, (uint64_t)idt64_isr_95, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 96, (uint64_t)idt64_isr_96, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 97, (uint64_t)idt64_isr_97, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 98, (uint64_t)idt64_isr_98, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 99, (uint64_t)idt64_isr_99, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 100, (uint64_t)idt64_isr_100, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 101, (uint64_t)idt64_isr_101, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 102, (uint64_t)idt64_isr_102, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 103, (uint64_t)idt64_isr_103, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 104, (uint64_t)idt64_isr_104, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 105, (uint64_t)idt64_isr_105, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 106, (uint64_t)idt64_isr_106, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 107, (uint64_t)idt64_isr_107, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 108, (uint64_t)idt64_isr_108, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 109, (uint64_t)idt64_isr_109, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 110, (uint64_t)idt64_isr_110, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 111, (uint64_t)idt64_isr_111, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 112, (uint64_t)idt64_isr_112, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 113, (uint64_t)idt64_isr_113, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 114, (uint64_t)idt64_isr_114, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 115, (uint64_t)idt64_isr_115, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 116, (uint64_t)idt64_isr_116, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 117, (uint64_t)idt64_isr_117, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 118, (uint64_t)idt64_isr_118, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 119, (uint64_t)idt64_isr_119, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 120, (uint64_t)idt64_isr_120, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 121, (uint64_t)idt64_isr_121, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 122, (uint64_t)idt64_isr_122, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 123, (uint64_t)idt64_isr_123, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 124, (uint64_t)idt64_isr_124, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 125, (uint64_t)idt64_isr_125, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 126, (uint64_t)idt64_isr_126, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 127, (uint64_t)idt64_isr_127, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 128, (uint64_t)idt64_isr_128, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 129, (uint64_t)idt64_isr_129, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 130, (uint64_t)idt64_isr_130, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 131, (uint64_t)idt64_isr_131, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 132, (uint64_t)idt64_isr_132, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 133, (uint64_t)idt64_isr_133, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 134, (uint64_t)idt64_isr_134, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 135, (uint64_t)idt64_isr_135, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 136, (uint64_t)idt64_isr_136, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 137, (uint64_t)idt64_isr_137, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 138, (uint64_t)idt64_isr_138, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 139, (uint64_t)idt64_isr_139, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 140, (uint64_t)idt64_isr_140, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 141, (uint64_t)idt64_isr_141, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 142, (uint64_t)idt64_isr_142, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 143, (uint64_t)idt64_isr_143, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 144, (uint64_t)idt64_isr_144, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 145, (uint64_t)idt64_isr_145, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 146, (uint64_t)idt64_isr_146, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 147, (uint64_t)idt64_isr_147, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 148, (uint64_t)idt64_isr_148, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 149, (uint64_t)idt64_isr_149, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 150, (uint64_t)idt64_isr_150, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 151, (uint64_t)idt64_isr_151, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 152, (uint64_t)idt64_isr_152, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 153, (uint64_t)idt64_isr_153, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 154, (uint64_t)idt64_isr_154, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 155, (uint64_t)idt64_isr_155, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 156, (uint64_t)idt64_isr_156, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 157, (uint64_t)idt64_isr_157, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 158, (uint64_t)idt64_isr_158, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 159, (uint64_t)idt64_isr_159, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 160, (uint64_t)idt64_isr_160, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 161, (uint64_t)idt64_isr_161, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 162, (uint64_t)idt64_isr_162, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 163, (uint64_t)idt64_isr_163, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 164, (uint64_t)idt64_isr_164, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 165, (uint64_t)idt64_isr_165, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 166, (uint64_t)idt64_isr_166, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 167, (uint64_t)idt64_isr_167, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 168, (uint64_t)idt64_isr_168, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 169, (uint64_t)idt64_isr_169, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 170, (uint64_t)idt64_isr_170, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 171, (uint64_t)idt64_isr_171, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 172, (uint64_t)idt64_isr_172, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 173, (uint64_t)idt64_isr_173, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 174, (uint64_t)idt64_isr_174, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 175, (uint64_t)idt64_isr_175, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 176, (uint64_t)idt64_isr_176, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 177, (uint64_t)idt64_isr_177, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 178, (uint64_t)idt64_isr_178, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 179, (uint64_t)idt64_isr_179, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 180, (uint64_t)idt64_isr_180, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 181, (uint64_t)idt64_isr_181, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 182, (uint64_t)idt64_isr_182, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 183, (uint64_t)idt64_isr_183, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 184, (uint64_t)idt64_isr_184, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 185, (uint64_t)idt64_isr_185, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 186, (uint64_t)idt64_isr_186, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 187, (uint64_t)idt64_isr_187, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 188, (uint64_t)idt64_isr_188, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 189, (uint64_t)idt64_isr_189, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 190, (uint64_t)idt64_isr_190, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 191, (uint64_t)idt64_isr_191, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 192, (uint64_t)idt64_isr_192, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 193, (uint64_t)idt64_isr_193, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 194, (uint64_t)idt64_isr_194, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 195, (uint64_t)idt64_isr_195, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 196, (uint64_t)idt64_isr_196, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 197, (uint64_t)idt64_isr_197, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 198, (uint64_t)idt64_isr_198, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 199, (uint64_t)idt64_isr_199, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 200, (uint64_t)idt64_isr_200, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 201, (uint64_t)idt64_isr_201, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 202, (uint64_t)idt64_isr_202, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 203, (uint64_t)idt64_isr_203, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 204, (uint64_t)idt64_isr_204, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 205, (uint64_t)idt64_isr_205, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 206, (uint64_t)idt64_isr_206, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 207, (uint64_t)idt64_isr_207, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 208, (uint64_t)idt64_isr_208, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 209, (uint64_t)idt64_isr_209, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 210, (uint64_t)idt64_isr_210, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 211, (uint64_t)idt64_isr_211, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 212, (uint64_t)idt64_isr_212, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 213, (uint64_t)idt64_isr_213, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 214, (uint64_t)idt64_isr_214, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 215, (uint64_t)idt64_isr_215, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 216, (uint64_t)idt64_isr_216, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 217, (uint64_t)idt64_isr_217, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 218, (uint64_t)idt64_isr_218, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 219, (uint64_t)idt64_isr_219, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 220, (uint64_t)idt64_isr_220, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 221, (uint64_t)idt64_isr_221, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 222, (uint64_t)idt64_isr_222, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 223, (uint64_t)idt64_isr_223, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 224, (uint64_t)idt64_isr_224, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 225, (uint64_t)idt64_isr_225, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 226, (uint64_t)idt64_isr_226, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 227, (uint64_t)idt64_isr_227, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 228, (uint64_t)idt64_isr_228, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 229, (uint64_t)idt64_isr_229, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 230, (uint64_t)idt64_isr_230, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 231, (uint64_t)idt64_isr_231, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 232, (uint64_t)idt64_isr_232, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 233, (uint64_t)idt64_isr_233, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 234, (uint64_t)idt64_isr_234, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 235, (uint64_t)idt64_isr_235, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 236, (uint64_t)idt64_isr_236, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 237, (uint64_t)idt64_isr_237, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 238, (uint64_t)idt64_isr_238, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 239, (uint64_t)idt64_isr_239, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 240, (uint64_t)idt64_isr_240, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 241, (uint64_t)idt64_isr_241, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 242, (uint64_t)idt64_isr_242, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 243, (uint64_t)idt64_isr_243, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 244, (uint64_t)idt64_isr_244, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 245, (uint64_t)idt64_isr_245, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 246, (uint64_t)idt64_isr_246, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 247, (uint64_t)idt64_isr_247, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 248, (uint64_t)idt64_isr_248, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 249, (uint64_t)idt64_isr_249, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 250, (uint64_t)idt64_isr_250, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 251, (uint64_t)idt64_isr_251, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 252, (uint64_t)idt64_isr_252, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 253, (uint64_t)idt64_isr_253, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 254, (uint64_t)idt64_isr_254, IDT64_ISR_INTERRUPT, 0);
    idt64_set_gate(idt_table, 255, (uint64_t)idt64_isr_255, IDT64_ISR_INTERRUPT, 0);
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
    idt_table[index].attr = type == IDT64_ISR_INTERRUPT ? 0x8E : 0x8F;
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