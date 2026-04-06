/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <kernel/vmm.h>
#include <kernel/state.h>
#include <kernel/msr.h>

#define LAPIC_REG_ID_OFFSET                 0x020
#define LAPIC_REG_VERSION_OFFSET            0x030
#define LAPIC_REG_TPR_OFFSET                0x080
#define LAPIC_REG_APR_OFFSET                0x090
#define LAPIC_REG_PPR_OFFSET                0x0A0
#define LAPIC_REG_EOI_OFFSET                0x0B0
#define LAPIC_REG_RRD_OFFSET                0x0C0
#define LAPIC_REG_LOGICAL_DEST_OFFSET       0x0D0
#define LAPIC_REG_DEST_FORMAT_OFFSET        0x0E0
#define LAPIC_REG_SPURIOUS_INT_VEC_OFFSET   0x0F0
#define LAPIC_REG_IN_SERVICE_OFFSET         0x100
#define LAPIC_REG_TRIGGER_MODE_OFFSET       0x180
#define LAPIC_REG_INT_REQUEST_OFFSET        0x200
#define LAPIC_REG_ERROR_STATUS              0x280
#define LAPIC_REG_LVT_CMCI_OFFSET           0x2F0
#define LAPIC_REG_INT_COMMAND_OFFSET        0x300
#define LAPIC_REG_LVT_TIMER_OFFSET          0x320
#define LAPIC_REG_LVT_THERMAL_SENSOR_OFFSET 0x330
#define LAPIC_REG_LVT_PERFMON_OFFSET        0x340
#define LAPIC_REG_LVT_LINT0_OFFSET          0x350
#define LAPIC_REG_LVT_LINT1_OFFSET          0x360
#define LAPIC_REG_LVT_ERROR_OFFSET          0x370
#define LAPIC_REG_INITIAL_COUNT_OFFSET      0x380
#define LAPIC_REG_CURRENT_COUNT_OFFSET      0x390
#define LAPIC_REG_DIVIDE_CONFIG_OFFSET      0x3E0

void lapic_init(GlobalState *kernel_state, CpuState *cpu_state);

uint32_t lapic_get_id();

static inline uint32_t lapic_read_register(void *lapic, uint16_t offset) {
    return *(volatile uint32_t*)((uintptr_t)lapic + offset);
}

static inline void lapic_write_register(void *lapic, uint16_t offset, uint32_t value) {
    *(volatile uint32_t*)((uintptr_t)lapic + offset) = value;
}

static inline void lapic_eoi(void *lapic) {
    lapic_write_register(lapic, LAPIC_REG_EOI_OFFSET, 0);
}

static inline void lapic_enable_svr(void *lapic, uint8_t vector) {
    lapic_write_register(
        lapic,
        LAPIC_REG_SPURIOUS_INT_VEC_OFFSET,
        vector | ((uint32_t)1 << 8)
    );
}

static inline void lapic_disable_svr(void *lapic) {
    lapic_write_register(
        lapic, 
        LAPIC_REG_SPURIOUS_INT_VEC_OFFSET, 
        lapic_read_register(lapic, LAPIC_REG_SPURIOUS_INT_VEC_OFFSET) & 0xFF  
    );
}

// ==== Timer ====

#define LAPIC_TIMER_MODE_ONESHOT      0x00
#define LAPIC_TIMER_MODE_PERIODIC     0x01
#define LAPIC_TIMER_MODE_TSC_DEADLINE 0x10

typedef enum {
    LAPIC_TIMER_DIV_2   = 0b0000,
    LAPIC_TIMER_DIV_4   = 0b0001,
    LAPIC_TIMER_DIV_8   = 0b0010,
    LAPIC_TIMER_DIV_16  = 0b0011,
    LAPIC_TIMER_DIV_32  = 0b1000,
    LAPIC_TIMER_DIV_64  = 0b1001,
    LAPIC_TIMER_DIV_128 = 0b1010,
    LAPIC_TIMER_DIV_1   = 0b1011,
} LapicDivisor;

static inline void lapic_timer_set_counter(void *lapic, uint32_t counter) {
    lapic_write_register(lapic, LAPIC_REG_INITIAL_COUNT_OFFSET, counter);
}

static inline void lapic_timer_init(void *lapic, const uint8_t vector, const bool masked, const uint8_t mode, const LapicDivisor divisor) {
    lapic_write_register(
        lapic,
        LAPIC_REG_DIVIDE_CONFIG_OFFSET,
        divisor
    );

    lapic_write_register(
        lapic,
        LAPIC_REG_LVT_TIMER_OFFSET,
        vector | ((uint32_t)masked << 16) | ((uint32_t)mode << 17)
    );
}

static inline uint32_t lapic_get_timer_counter(void *lapic) {
    return lapic_read_register(lapic, LAPIC_REG_CURRENT_COUNT_OFFSET);
}

// Defines when calibration timer hits what milliseconds
// will be used to determine LAPIC Timer speed
#define LAPIC_TIMER_CALIBRATION_BASE    10
#define LAPIC_TIMER_CALIBRATION_SAMPLES 10

void lapic_timer_calibrate(void);