/*
 * Pristine
 * lapic: Local Advanced Interrupt Controller definitions
 * SPDX-License-Identifier: MIT
 */

#include <kernel/lapic.h>
#include <kernel/vmm.h>
#include <kernel/mmio.h>

void lapic_init(GlobalState *global_state, CpuState *cpu_state) {
    const uintptr_t lapic_cpu_offset = cpu_state->id * VMM_DEFAULT_PAGE_SIZE;
    const uint64_t lapic_msr = rdmsr(MSR_REG_LAPIC_BASE);
    const uintptr_t lapic_phys = lapic_msr & 0xFFFFFFFF000ULL;
    vmm_map(
        global_state->pml4,
        lapic_phys,
        MMIO_VIRT_LAPIC_START + lapic_cpu_offset,
        VMM_FLAGS_KERNEL_DATA | VMM_FLAGS_NO_CACHE
    );
    cpu_state->lapic = (void*)(MMIO_VIRT_LAPIC_START + lapic_cpu_offset);
}