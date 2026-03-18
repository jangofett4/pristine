/*
 * Pristine
 * atapio: ATA PIO disk access functions
 * SPDX-License-Identifier: MIT
 */

#include "kernel/panic.h"
#include <drivers/storage/ata/atapio.h>
#include <common/io.h>
#include <common/disk.h>

static uint16_t _ata_buffer_identify[256] = {0};

AtaPioStatus ata_pio_get_status() {
    AtaPioStatus status;
    status.raw = io_inb(ATA_PIO_PORT_COMMAND);
    return status;
}

void ata_pio_delay(void) {
    io_inb(0x3F6);
    io_inb(0x3F6);
    io_inb(0x3F6);
    io_inb(0x3F6);
}

void ata_pio_set_disk(uint8_t disk) {
    io_outb(ATA_PIO_PORT_DRIVEHEAD, disk);
    ata_pio_delay();
}

void ata_pio_zero(void) {
    io_outb(ATA_PIO_PORT_SECTORCOUNT, 0x00);
    io_outb(ATA_PIO_PORT_LBALOW, 0x00);
    io_outb(ATA_PIO_PORT_LBAMID, 0x00);
    io_outb(ATA_PIO_PORT_LBAHIGH, 0x00);
}

uint16_t ata_pio_read_dataport(void) {
    return io_inw(ATA_PIO_PORT_DATAREGISTER);
}

AtaPioStatus ata_pio_identify(void) {
    io_outb(ATA_PIO_PORT_COMMAND, ATA_PIO_CMD_IDENTIFY);
    AtaPioStatus status = ata_pio_get_status();
    if (status.raw == 0)
        return status;
    while ((status = ata_pio_get_status()).flags.bsy) {
        if (status.flags.err) {
            return status;
        }
    }

    if (io_inb(ATA_PIO_PORT_LBAMID) != 0 || io_inb(ATA_PIO_PORT_LBAHIGH) != 0) {
        status.raw = 0;  // not plain ATA
        return status;
    }

    while (!(status = ata_pio_get_status()).flags.drq);

    for (size_t i = 0; i < 256; i++)
        _ata_buffer_identify[i] = ata_pio_read_dataport();

    return status;
}

void ata_pio_get_serial_number(char buf[21]) {
    for (size_t i = 0; i < 10; i++) {
        buf[i * 2 + 0] = (_ata_buffer_identify[10 + i] >> 8) & 0xFF;
        buf[i * 2 + 1] =  _ata_buffer_identify[10 + i]       & 0xFF;
    }
    buf[20] = '\0';
}

void ata_pio_get_model_number(char buf[41]) {
    for (size_t i = 0; i < 20; i++) {
        buf[i * 2 + 0] = (_ata_buffer_identify[27 + i] >> 8) & 0xFF;
        buf[i * 2 + 1] =  _ata_buffer_identify[27 + i]       & 0xFF;
    }
    buf[40] = '\0';
}

uint32_t ata_pio_get_max_sectors(void) {
    return ((uint32_t)_ata_buffer_identify[61] << 16) | (uint32_t)_ata_buffer_identify[60];
}

uint8_t ata_pio_drive_ready(void) {
    AtaPioStatus status = ata_pio_get_status();
    return (!status.flags.bsy) & status.flags.rdy;
}

AtaPioReadStatus ata_pio_readsector(uint32_t lba, uint8_t *buf) {
    io_outb(ATA_PIO_PORT_DRIVEHEAD, 0xE0 | ((lba >> 24) & 0x0F));
    ata_pio_delay();
    
    io_outb(ATA_PIO_PORT_FEATURES, 0x00);
    io_outb(ATA_PIO_PORT_SECTORCOUNT, 0x01);
    io_outb(ATA_PIO_PORT_LBALOW,  (lba >>  0) & 0xFF);
    io_outb(ATA_PIO_PORT_LBAMID,  (lba >>  8) & 0xFF);
    io_outb(ATA_PIO_PORT_LBAHIGH, (lba >> 16) & 0xFF);
    io_outb(ATA_PIO_PORT_COMMAND, ATA_PIO_CMD_READ);
    ata_pio_delay();
    AtaPioStatus status = ata_pio_get_status();
    AtaPioReadStatus readstatus;
    do {
        status = ata_pio_get_status();
        if (status.flags.err) break;
    } while (status.flags.bsy || !status.flags.drq);

    if (status.flags.err) {
        status.raw = 0;
        readstatus.status = status;
        return readstatus;
    }

    uint16_t *buf16 = (uint16_t*)buf;
    for (size_t i = 0; i < 256; i++)
        buf16[i] = ata_pio_read_dataport();

    readstatus.status = status;
    readstatus.data = (uint8_t*)buf;
    return readstatus;
}

AtaPioReadStatus ata_pio_readsectors(uint32_t lba, uint8_t sectors_to_read, uint8_t *buf) {
    AtaPioReadStatus readstatus;
    AtaPioStatus status;

    if (sectors_to_read > DISK_READ_MAX_BLOCKS) {
        readstatus.data = NULL;
        return readstatus;
    }
    
    io_outb(ATA_PIO_PORT_DRIVEHEAD, 0xE0 | ((lba >> 24) & 0x0F));
    ata_pio_delay();
    
    io_outb(ATA_PIO_PORT_FEATURES, 0x00);
    io_outb(ATA_PIO_PORT_SECTORCOUNT, sectors_to_read);
    io_outb(ATA_PIO_PORT_LBALOW,  (lba >>  0) & 0xFF);
    io_outb(ATA_PIO_PORT_LBAMID,  (lba >>  8) & 0xFF);
    io_outb(ATA_PIO_PORT_LBAHIGH, (lba >> 16) & 0xFF);
    io_outb(ATA_PIO_PORT_COMMAND, ATA_PIO_CMD_READ);
    ata_pio_delay();

    uint16_t *buf16 = (uint16_t*)buf;
    for (uint8_t sector = 0; sector < sectors_to_read; sector++) {
        status = ata_pio_get_status();
        do {
            status = ata_pio_get_status();
            if (status.flags.err) break;
        } while (status.flags.bsy || !status.flags.drq);

        if (status.flags.err) {
            status.raw = 0;
            readstatus.status = status;
            return readstatus;
        }

        for (size_t i = 0; i < 256; i++)
        {
            status = ata_pio_get_status();
            if (!status.flags.drq || status.flags.err) {
                KPANIC("ata_pio_readsectors: lost DRQ at sector=%d i=%d status=%02x", sector, i, status.raw);
            }
            buf16[i + (sector * 256)] = ata_pio_read_dataport();
        }
    }

    readstatus.status = status;
    readstatus.data = (uint8_t*)buf;
    return readstatus;
}
    
uint8_t ata_pio_disk_ops_read(uint32_t lba, size_t sector_count, uint8_t *buf) {
    AtaPioReadStatus status = ata_pio_readsectors(lba, sector_count, buf);
    if (status.status.raw == 0)
        return 0;
    return 1;
}

uint8_t ata_pio_disk_ops_write(uint32_t lba, size_t sector_count, const uint8_t *buf) {
    return 0;
}

DiskOpsVtable ata_pio_get_disk_ops() {
    DiskOpsVtable vtable = {
        .read = ata_pio_disk_ops_read,
        .write = ata_pio_disk_ops_write
    };
    return vtable;
}