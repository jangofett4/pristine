/*
 * Pristine
 * lapic: Local Advanced Interrupt Controller definitions
 * SPDX-License-Identifier: MIT
 */

#include <kernel/cpu.h>
#include <kernel/lapic.h>
#include <kernel/vmm.h>
#include <kernel/mmio.h>
#include <stdint.h>

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
    cpu_state->lapic_calibrated = false;
    cpu_state->lapic_timer_speed = 0;
    cpu_state->lapic_samples = 0;
}

void lapic_timer_calibrate(void) {
    CpuState *cpu_state = get_cpu_state();
    if (cpu_state->lapic_samples == 0) { // Skip first tick
        lapic_timer_set_counter(cpu_state->lapic, UINT32_MAX);
        cpu_state->lapic_samples++;
        return;
    }

    cpu_state->lapic_samples++;

    if (cpu_state->lapic_samples >= LAPIC_TIMER_CALIBRATION_SAMPLES) {
        cpu_state->lapic_calibrated = true;
        const uint32_t current_count = lapic_read_register(cpu_state->lapic, LAPIC_REG_CURRENT_COUNT_OFFSET);
        cpu_state->lapic_timer_speed = (UINT32_MAX - current_count) / (LAPIC_TIMER_CALIBRATION_SAMPLES - 1);
        lapic_eoi(cpu_state->lapic);
    }
}