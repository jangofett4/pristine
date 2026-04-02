/*
 * Pristine
 * kernel: kernel entry point
 * SPDX-License-Identifier: MIT
 */

#include <kernel/syscall.h>
#include <uapi/syscall.h>
#include <stdint.h>

#include <pristine.h>
#include <kernel/panic.h>
#include <kernel/kernel.h>
#include <kernel/state.h>
#include <kernel/global.h>
#include <kernel/pmm.h>
#include <kernel/vmm.h>
#include <kernel/process.h>
#include <kernel/msr.h>
#include <common/crc32.h>
#include <common/elf.h>
#include <common/memmap.h>
#include <common/common.h>
#include <common/pic.h>
#include <common/gdt.h>
#include <common/disk.h>
#include <common/idt64.h>
#include <common/arena.h>
#include <common/serial.h>
#include <common/string.h>
#include <common/bootinfo.h>

#include <common/bsfs/bsfs.h>
#include <common/bsfs/bsfs_ops.h>
#include <common/bsfs/bsfs_defaults.h>

#include <bitmap.h>

#include <drivers/storage/ata/atapio.h>
#include <drivers/video/video.h>

#include <printf.h>

KernelState kernel_state;

void kmain(uint64_t bootinfo_addr) {
    // verify we're running where we think we are
    uint64_t rip;
    __asm__ volatile("lea (%%rip), %0" : "=r"(rip));
    if (rip < KERNEL_BASE_VIRT || rip > KERNEL_BASE_VIRT + 0x200000) {
        KPANIC_SILENT();
    }

    arena_reset();

    bootinfo_init(&kernel_state.bootinfo, bootinfo_addr);

    // ======== IDT64 ========
    idt64_disable_interrupts();
    idt64_init();
    idt64_reload_idtr();

    // ======== PIC ========

    pic_init();

    idt64_enable_interrupts();

    // pic_unmask_irq(1);

    // ======== Globals ========

    serial_init(&kernel_state.serial, 0x3F8);
    serial_set_default(&kernel_state.serial);

    video_init(&kernel_state.video, &kernel_state.bootinfo.vesa_vbe_mode_info);
    video_set_default(&kernel_state.video);

    kernel_state.memory_bitmap = kernel_state.bootinfo.memory_bitmap;
    pmm_init(kernel_state.memory_bitmap, kernel_state.bootinfo.memory_bitmap_size);
    bitmap_clear_all(kernel_state.memory_bitmap, kernel_state.bootinfo.memory_bitmap_size);

    // ======== Memory Map ========

    if (kernel_state.bootinfo.memory_map_count > MEMMAP_MAX_ITEMS) {
        KPANIC("kernel: memory map exceeds maximum items of %i, possibly malformed memory", MEMMAP_MAX_ITEMS);
    }

    kernel_state.system_memory = memmap_bitmap_init(kernel_state.bootinfo.memory_map, kernel_state.bootinfo.memory_map_count, kernel_state.memory_bitmap);

    uint64_t memory_bitmap_start_phys = ((uint64_t)kernel_state.bootinfo.memory_bitmap) - PMM_HHDM_START;
    uint64_t memory_bitmap_end_phys   = memory_bitmap_start_phys + kernel_state.bootinfo.memory_bitmap_size;

    uint64_t memory_bitmap_start_phys_aligned = ALIGN_UP(memory_bitmap_start_phys, VMM_DEFAULT_PAGE_SIZE);
    uint64_t memory_bitmap_end_phys_aligned   = ALIGN_DOWN(memory_bitmap_end_phys, VMM_DEFAULT_PAGE_SIZE);

    printf_("Memory Bitmap Start: %lu, End: %lu (Size: %lu, Covers %lu MiB)\n", 
        memory_bitmap_start_phys_aligned, 
        memory_bitmap_end_phys_aligned, 
        memory_bitmap_end_phys - memory_bitmap_start_phys, 
        (memory_bitmap_end_phys - memory_bitmap_start_phys) * 8 * VMM_DEFAULT_PAGE_SIZE / 1024 / 1024
    );

    // mark memory bitmap as used
    for (size_t i = memory_bitmap_start_phys_aligned; i < memory_bitmap_end_phys_aligned; i += VMM_DEFAULT_PAGE_SIZE) {
        bitmap_set(kernel_state.memory_bitmap, i / VMM_DEFAULT_PAGE_SIZE);
    }

    // mark kernel memory area as used
    for (size_t i = KERNEL_BASE_PHYS; i < KERNEL_END_PHYS; i += VMM_DEFAULT_PAGE_SIZE) {
        bitmap_set(kernel_state.memory_bitmap, i / VMM_DEFAULT_PAGE_SIZE);
    }

    // mark dangerous regions as used, no need to allocate 1 MiB of space here, but just being careful here
    for (size_t i = 0; i < 0x100000; i += VMM_DEFAULT_PAGE_SIZE) {
        bitmap_set(kernel_state.memory_bitmap, i / VMM_DEFAULT_PAGE_SIZE);
    }

    printf_("Total Usable Memory: %zu MiB\n", kernel_state.system_memory / 1024 / 1024);

    // ======== GDT, TSS ========

    kernel_state.gdt = phys_to_virt(pmm_alloc());
    kernel_state.tss = phys_to_virt(pmm_alloc());

    printf_("GDT %p\n", kernel_state.gdt);
    printf_("TSS %p\n", kernel_state.tss);

    memcpy(kernel_state.gdt, kernel_state.bootinfo.gdt, kernel_state.bootinfo.gdt_entries * sizeof(uint64_t));
    
    kernel_state.tss[0].rsp0 = (uint64_t)__kernel_rsp0_start;
    kernel_state.tss[0].ist1 = (uint64_t)__kernel_ist1_start;

    printf_("TSS.RSP0 %p\n", __kernel_rsp0_start);
    printf_("TSS.IST1 %p\n", __kernel_ist1_start);

    // gdt[0] -> null (offset 0)
    // gdt[1] -> kernel code (offset 0x08)
    // gdt[2] -> kernel data (offset 0x10)
    gdt_set_entry(kernel_state.gdt, 3, 0, UINT32_MAX, 0xF2, 0xA); // User data
    gdt_set_entry(kernel_state.gdt, 4, 0, UINT32_MAX, 0xFA, 0xA); // User code
    gdt_set_tss_entry(kernel_state.gdt, 5, (void*)kernel_state.tss, 0x89, 0, sizeof(TSSEntry) - 1);
    gdt_load_gdtr(kernel_state.gdt, 7);
    gdt_reload_cs(0x08);
    gdt_reload_segments(0x10);
    gdt_load_tss(0x28);

    // ======== Paging ========

    for (uint64_t i = kernel_state.bootinfo.raw.pml4_address; i < (kernel_state.bootinfo.raw.pml4_address + kernel_state.bootinfo.raw.page_table_count * VMM_DEFAULT_PAGE_SIZE); i += VMM_DEFAULT_PAGE_SIZE) {
        bitmap_set(kernel_state.memory_bitmap, i / VMM_DEFAULT_PAGE_SIZE);
    }

    vmm_init();

    // HHDM
    for (size_t i = 0; i < 512; i++) {
        vmm_map_huge(
            vmm_pml4, 
            i * VMM_HUGE_PAGE_SIZE, 
            PMM_HHDM_START + (i * VMM_HUGE_PAGE_SIZE), 
            VMM_FLAGS_KERNEL_CODE
        );
    }

    // Kernel
    for (uint64_t off = 0; off < 0x200000; off += VMM_DEFAULT_PAGE_SIZE) {
        vmm_map(
            vmm_pml4,
            KERNEL_BASE_PHYS + off,
            KERNEL_BASE_VIRT + off,
            VMM_FLAGS_KERNEL_CODE
        );
    }

    // GDT and TSS
    // vmm_map(__vmm_pml4, virt_to_phys(__global_gdt), (uint64_t)__global_gdt, VMM_FLAGS_KERNEL_DATA);
    // vmm_map(__vmm_pml4, virt_to_phys(__global_tss), (uint64_t)__global_tss, VMM_FLAGS_KERNEL_DATA);

    // Stack, RSP0, IST1

    for (uintptr_t i = 0; i < KERNEL_STACK_SIZE; i += VMM_DEFAULT_PAGE_SIZE) {
        const uintptr_t addr = (uintptr_t)__kernel_stack_start - KERNEL_STACK_SIZE + i;
        vmm_unmap(vmm_pml4, addr);
        vmm_map(vmm_pml4, kernel_virt_to_phys((void*)addr), addr, VMM_FLAGS_KERNEL_DATA);
    }

    for (uintptr_t i = 0; i < KERNEL_RSP0_SIZE; i += VMM_DEFAULT_PAGE_SIZE) {
        const uintptr_t addr = (uintptr_t)__kernel_rsp0_start - KERNEL_RSP0_SIZE + i;
        vmm_unmap(vmm_pml4, addr);
        vmm_map(vmm_pml4, kernel_virt_to_phys((void*)addr), addr, VMM_FLAGS_KERNEL_DATA);
    }

    for (uintptr_t i = 0; i < KERNEL_IST1_SIZE; i += VMM_DEFAULT_PAGE_SIZE) {
        const uintptr_t addr = (uintptr_t)__kernel_ist1_start - KERNEL_IST1_SIZE + i;
        vmm_unmap(vmm_pml4, addr);
        vmm_map(vmm_pml4, kernel_virt_to_phys((void*)addr), addr, VMM_FLAGS_KERNEL_DATA);
    }

    vmm_unmap(vmm_pml4, (uintptr_t)__kernel_stack_guard);
    vmm_unmap(vmm_pml4, (uintptr_t)__kernel_rsp0_guard);
    vmm_unmap(vmm_pml4, (uintptr_t)__kernel_ist1_guard);

    idt64_disable_interrupts();
    vmm_switch(vmm_pml4);
    idt64_enable_interrupts();

    // ======== Syscall ========

    uint64_t msr_star = ((0x10ULL << 48) | (0x08ULL << 32)) & (0xFFFFFFFF00000000);
    uint64_t msr_lstar = (uint64_t)(uintptr_t)syscall_stub;
    //                    v SF bit    v IF bit    v DF bit
    uint64_t msr_sfmask = 1ULL << 8 | 1ULL << 9 | 1ULL << 10;
    wrmsr(MSR_REG_STAR, msr_star);
    wrmsr(MSR_REG_LSTAR, msr_lstar);
    wrmsr(MSR_REG_SFMASK, msr_sfmask);
    wrmsr(MSR_REG_GSBASE, (uint64_t)(uintptr_t)&kernel_state);
    wrmsr(MSR_REG_KERNELGSBASE, 0);

    syscall_init(&kernel_state);

    // ======== BSFS ========

    kernel_state.disk_ops = ata_pio_get_disk_ops();

    const uint32_t bsfs_offset = PRISTINE_BSFS_OFFSET * 4096;

    uint8_t *tmp_diskbuf = arena_alloc(512, 1);
    if (!kernel_state.disk_ops.read(bsfs_offset / ATA_PIO_SECTOR_SIZE, 1, tmp_diskbuf)) {
        KPANIC("unable to read disk");
    }
    memcpy(&kernel_state.bsfs_header, tmp_diskbuf, sizeof(BsfsHeader));
    arena_reset();

    if (kernel_state.bsfs_header.block_size != PRISTINE_BSFS_BLOCKSIZE) {
        KPANIC("invalid filesystem block size, expected %i, got %u", PRISTINE_BSFS_BLOCKSIZE, kernel_state.bsfs_header.block_size);
    }

    _Static_assert(ATA_PIO_SECTOR_SIZE * DISK_READ_MAX_BLOCKS <= 4096, "disk scratch buffer doesn't fit inside a single page");

    kernel_state.bsfs_context = (BsfsContext){
        .disk_ops = &kernel_state.disk_ops,
        .header = &kernel_state.bsfs_header,
        .phys_sector_size = ATA_PIO_SECTOR_SIZE,
        .scratch_buf = phys_to_virt(pmm_alloc()),
        .scratch_buf_size = ATA_PIO_SECTOR_SIZE * DISK_READ_MAX_BLOCKS
    };

    printf_("BSFS Version %x\n", kernel_state.bsfs_header.version);

    BsfsInode hello_inode;
    BsfsFile hello_file = {
        .inode = &hello_inode,
        .buf = (uint8_t*)arena_alloc(kernel_state.bsfs_header.block_size, 1),
        .bufsize = kernel_state.bsfs_header.block_size,
        .l1_table = (uint32_t*)arena_alloc(kernel_state.bsfs_header.block_size, 1)
    };
    int read = 0;
    if ((read = bsfs_fopen(&kernel_state.bsfs_context, "/bin/hello.elf", &hello_file)) < 0) {
        const char* err = bsfs_strerror(read);
        KPANIC("kernel: cannot open hello.elf: %s", err);
    }
    
    if ((read = bsfs_verify_checksum(&kernel_state.bsfs_context, &hello_file, (uint8_t*)arena_alloc(512, 1), 512)) < 0) {
        const char* err = bsfs_strerror(read);
        KPANIC("kernel: bsfs_verify_checksum failed: %s", err);
    }

    bsfs_fseeko(&kernel_state.bsfs_context, &hello_file, 0, BSFS_FSEEKO_SET);

    uint64_t  hello_pml4_phys = pmm_alloc();
    uint64_t *hello_pml4_hhdm = phys_to_virt(hello_pml4_phys);
    memset(hello_pml4_hhdm, 0, VMM_DEFAULT_PAGE_SIZE);
    Process hello_process = {
        .pml4 = hello_pml4_hhdm,
    };

    Elf64Ehdr hello_file_hdr;
    elf64_load_executable(&kernel_state.bsfs_context, &hello_file, hello_pml4_hhdm, &hello_file_hdr);
    
    // Userspace stack allocation
    size_t num_process_stack_pages = PROCESS_USERSPACE_DEFAULT_STACK_SIZE / VMM_DEFAULT_PAGE_SIZE;
    for (size_t i = 1; i <= num_process_stack_pages; i++) {
        uint64_t phys_page = pmm_alloc();
        uint64_t virt_addr = PROCESS_USERSPACE_VIRT_STACK_TOP - (i * VMM_DEFAULT_PAGE_SIZE);
        vmm_map(hello_pml4_hhdm, phys_page, virt_addr, VMM_FLAGS_USER_DATA);
    }

    // Dedicated kernel stack allocation
    size_t num_process_kernel_stack_pages = PROCESS_USERSPACE_DEFAULT_KERNEL_STACK_SIZE / VMM_DEFAULT_PAGE_SIZE;
    for (size_t i = 1; i <= num_process_kernel_stack_pages; i++) {
        uint64_t phys_page = pmm_alloc();
        uint64_t virt_addr = PROCESS_USERSPACE_VIRT_KERNEL_STACK_TOP - (i * VMM_DEFAULT_PAGE_SIZE);
        vmm_map(hello_pml4_hhdm, phys_page, virt_addr, VMM_FLAGS_KERNEL_DATA);
    }

    hello_process.entry = hello_file_hdr.e_entry;
    hello_process.stack_top = PROCESS_USERSPACE_VIRT_STACK_TOP;
    hello_process.kernel_stack_top = PROCESS_USERSPACE_VIRT_KERNEL_STACK_TOP;

    // "Copy" over HHDM & kernel mapping
    hello_pml4_hhdm[256] = vmm_pml4[256];
    hello_pml4_hhdm[511] = vmm_pml4[511];

    // Setup "context switch"
    kernel_state.kernel_stack_top = hello_process.kernel_stack_top;
    kernel_state.user_stack_rsp = hello_process.stack_top;
    kernel_state.current_process = &hello_process;
    
    printf_("hello.elf Entry: 0x%016x\n", hello_file_hdr.e_entry);

    process_start_trampoline(&hello_process);

    vmm_destroy(hello_pml4_hhdm);
    pmm_free(hello_pml4_phys);
    arena_reset();

    printf_("We good.\n");

    while(1);
}