/*
 * Pristine
 * elf: ELF handling functions and structures
 * SPDX-License-Identifier: MIT
 */

#include <common/elf.h>
#include <common/common.h>
#include <kernel/vmm.h>
#include <common/string.h>

const char *elf64_strerror(int err) {
    switch (err) {
        #define ELF_CASE(name, code, msg) case code: return msg;
        ELF_ERRORS(ELF_CASE)
        #undef ELF_CASE
        default: return "unknown error";
    }
}

// TODO: In case of bsfs related errors, instead of returning a generic error code
//       returning BSFS error code (or some sort of masked version of it) would make
//       it possible to print the actual error in the caller

int elf64_load_executable(BsfsContext *context, BsfsFile *file, uint64_t *pml4, Elf64Ehdr *out_header) {
    int read = 0;
    if ((read = bsfs_fread(context, out_header, sizeof(Elf64Ehdr), 1, file)) < 0) {
        return ELF_LOAD_READ_ERROR;
    }

    if (!elf64_check_magic(out_header)) {
        return ELF_LOAD_INVALID_ELF_MAGIC;
    }

    if (out_header->e_ident[4] != 0x02) {
        return ELF_LOAD_NOT_ELF64;
    }

    if (out_header->e_ident[5] != 0x01) {
        return ELF_LOAD_NOT_LITTLE_ENDIAN;
    }

    if (out_header->e_type != 0x02) {
        return  ELF_LOAD_NOT_EXECUTABLE;
    }

    if (out_header->e_phentsize != sizeof(Elf64Phdr)) {
        return ELF_LOAD_PHENTSIZE_MISMATCH;
    }

    if (bsfs_fseeko(context, file, out_header->e_phoff, BSFS_FSEEKO_SET) < 0) {
        return ELF_LOAD_MALFORMED;
    }

    for (size_t ph = 0; ph < out_header->e_phnum; ph++) {
        Elf64Phdr phdr;
        uint64_t ph_offset = out_header->e_phoff + (ph * out_header->e_phentsize);

        if (bsfs_fseeko(context, file, ph_offset, BSFS_FSEEKO_SET)) {
            return ELF_LOAD_SEEK_ERROR;
        }
        
        if ((read = bsfs_fread(context, &phdr, sizeof(Elf64Phdr), 1, file)) < 0) {
            return ELF_LOAD_READ_ERROR;
        }
        if (phdr.p_filesz > phdr.p_memsz) {
            return ELF_LOAD_FILESZ_EXCEEDS_MEMSZ;
        }

        if (phdr.p_type == PT_LOAD) {
            uint64_t virt_start = ALIGN_DOWN(phdr.p_vaddr, VMM_DEFAULT_PAGE_SIZE);
            uint64_t virt_end   = ALIGN_UP(phdr.p_vaddr + phdr.p_memsz, VMM_DEFAULT_PAGE_SIZE);
            int64_t remaining   = phdr.p_filesz;

            if ((read = bsfs_fseeko(context, file, phdr.p_offset, BSFS_FSEEKO_SET)) < 0) {
                return ELF_LOAD_SEEK_ERROR;
            }

            for (uint64_t addr = virt_start; addr < virt_end; addr += VMM_DEFAULT_PAGE_SIZE) {
                int offset = addr == virt_start ? phdr.p_vaddr % VMM_DEFAULT_PAGE_SIZE : 0;
                read = 0;
                uint64_t phys_page = pmm_alloc();
                vmm_map(pml4, phys_page, addr, VMM_FLAGS_USER_CODE);
                uint8_t *virt_page = (uint8_t*)phys_to_virt(phys_page);
                memset(virt_page, 0, VMM_DEFAULT_PAGE_SIZE);
                if (remaining > 0) {
                    if (remaining < VMM_DEFAULT_PAGE_SIZE) {
                        if ((read = bsfs_fread(context, virt_page + offset, 1, remaining - offset, file)) < 0) {
                            return ELF_LOAD_READ_ERROR;
                        }
                    } else {
                        if ((read = bsfs_fread(context, virt_page + offset, 1, VMM_DEFAULT_PAGE_SIZE - offset, file)) < 0) {
                            return ELF_LOAD_READ_ERROR;
                        }
                    }
                    remaining -= read;
                }
            }
        }
    }
    
    return 1;
}
