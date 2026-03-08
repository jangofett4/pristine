/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include "stage2_kstdint.h"
#include "stage2_disk.h"

#define ATA_PIO_PORT_DATAREGISTER 0x1F0
#define ATA_PIO_PORT_FEATURES     0x1F1
#define ATA_PIO_PORT_SECTORCOUNT  0x1F2
#define ATA_PIO_PORT_LBALOW       0x1F3
#define ATA_PIO_PORT_LBAMID       0x1F4
#define ATA_PIO_PORT_LBAHIGH      0x1F5
#define ATA_PIO_PORT_DRIVEHEAD    0x1F6
#define ATA_PIO_PORT_COMMAND      0x1F7

#define ATA_PIO_DISK_MASTER 0xA0
#define ATA_PIO_DISK_SLAVE  0xB0

#define ATA_PIO_CMD_READ        0x20
#define ATA_PIO_CMD_WRITE       0x30
#define ATA_PIO_CMD_IDENTIFY    0xEC
#define ATA_PIO_CMD_INITPARAM   0x91
#define ATA_PIO_CMD_CACHEFLUSH  0xE7

typedef union {
    uint8_t raw;
    struct {
        uint8_t err  : 1; // Error
        uint8_t idx  : 1; // Index
        uint8_t corr : 1; // Corrected data
        uint8_t drq  : 1; // Data request
        uint8_t srv  : 1; // Service
        uint8_t df   : 1; // Drive fault
        uint8_t rdy  : 1; // Ready
        uint8_t bsy  : 1; // Busy
    } __attribute__((packed)) flags;
} AtaPioStatus;

typedef union {
    uint8_t raw;
    struct {
        uint8_t amnf    : 1;
        uint8_t tk0nf   : 1;
        uint8_t abrt    : 1;
        uint8_t unused3 : 1;
        uint8_t idnf    : 1;
        uint8_t unused5 : 1;
        uint8_t unc     : 1;
        uint8_t bbk     : 1;
    } __attribute__((packed)) flags;
} AtaPioError;

typedef struct {
    AtaPioStatus status;
    uint8_t *data;
} AtaPioReadStatus;

AtaPioStatus ata_pio_get_status();
void ata_pio_set_disk(uint8_t disk);
void ata_pio_delay(void);
void ata_pio_zero(void);
AtaPioStatus ata_pio_identify(void);
uint16_t ata_pio_read_dataport(void);

void ata_pio_get_serial_number(char buf[21]);
void ata_pio_get_model_number(char buf[41]);
uint32_t ata_pio_get_max_sectors(void);
uint8_t ata_pio_drive_ready(void);

DiskOpsVtable ata_pio_get_disk_ops();

AtaPioReadStatus ata_pio_readsector(uint32_t lba, uint8_t *buf);
AtaPioReadStatus ata_pio_readsectors(uint32_t lba, uint8_t sectors_to_read, uint8_t *buf);