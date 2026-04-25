/*
 * Pristine
 * elf: ELF handling functions and structures
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <common/bsfs/bsfs_ops.h>

#include <stdint.h>

#define PT_LOAD 0x01

#define ELF_ERRORS(X) \
    X(ELF_LOAD_READ_ERROR,           -1,  "unable to read file")\
    X(ELF_LOAD_SEEK_ERROR,           -2,  "unable to seek in file")\
    X(ELF_LOAD_INVALID_ELF_MAGIC,    -3,  "invalid elf magic")\
    X(ELF_LOAD_NOT_ELF64,            -4,  "file is not ELF64 compiled")\
    X(ELF_LOAD_NOT_LITTLE_ENDIAN,    -5,  "file is not little endian ELF64")\
    X(ELF_LOAD_NOT_EXECUTABLE,       -6,  "file is not a executable ELF")\
    X(ELF_LOAD_MALFORMED,            -7,  "file contains a malformed ELF header")\
    X(ELF_LOAD_FILESZ_EXCEEDS_MEMSZ, -8,  "p_filesz exceeds p_memsz in PT_LOAD segment")\
    X(ELF_LOAD_PHENTSIZE_MISMATCH,   -9,  "e_phentsize is not equal to size of Elf64Phdr")

#define ELF_DEFINE(name, code, msg) static const int name = code;
ELF_ERRORS(ELF_DEFINE);

#undef BSFS_DEFINE

typedef struct {
    uint8_t  e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint64_t e_entry;
    uint64_t e_phoff;
    uint64_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} __attribute__((packed)) Elf64Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_flags;
    uint64_t p_offset;
    uint64_t p_vaddr;
    uint64_t p_paddr;
    uint64_t p_filesz;
    uint64_t p_memsz;
    uint64_t p_align;
} __attribute__((packed)) Elf64Phdr;

static inline bool elf64_check_magic(const Elf64Ehdr *hdr) {
    return hdr->e_ident[0] == 0x7F &&
           hdr->e_ident[1] == 0x45 &&
           hdr->e_ident[2] == 0x4C &&
           hdr->e_ident[3] == 0x46  ;
}

typedef struct {
    int       result;
    uintptr_t top;
} Elf64LoadResult;

// Loads given file 
Elf64LoadResult elf64_load_executable(BsfsContext *context, BsfsFile *file, uint64_t *pml4, Elf64Ehdr *out_header);
const char *elf64_strerror(int err);