/*
 * Pristine
 * stage2_ata_pio - ATA PIO disk access functions
 * SPDX-License-Identifier: MIT
 */

#include "stage2_ata_pio.h"
#include "stage2_disk.h"
#include "stage2_common.h"
#include "stage2_memory.h"
#include "stage2_io.h"
#include "printf.h"

static uint16_t _ata_buffer_identify[256] = {0};
// static uint16_t _ata_buffer_read[ATA_PIO_READ_MAXSECTORS / 2 * 256] = {0};

ata_pio_status_t ata_pio_get_status() {
    ata_pio_status_t status;
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

ata_pio_status_t ata_pio_identify(void) {
    io_outb(ATA_PIO_PORT_COMMAND, ATA_PIO_CMD_IDENTIFY);
    ata_pio_status_t status = ata_pio_get_status();
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
    ata_pio_status_t status = ata_pio_get_status();
    return (!status.flags.bsy) & status.flags.rdy;
}

ata_pio_read_status_t ata_pio_readsector(uint32_t lba, uint8_t *buf) {
    io_outb(ATA_PIO_PORT_DRIVEHEAD, 0xE0 | ((lba >> 24) & 0x0F));
    ata_pio_delay();
    
    io_outb(ATA_PIO_PORT_FEATURES, 0x00);
    io_outb(ATA_PIO_PORT_SECTORCOUNT, 0x01);
    io_outb(ATA_PIO_PORT_LBALOW,  (lba >>  0) & 0xFF);
    io_outb(ATA_PIO_PORT_LBAMID,  (lba >>  8) & 0xFF);
    io_outb(ATA_PIO_PORT_LBAHIGH, (lba >> 16) & 0xFF);
    io_outb(ATA_PIO_PORT_COMMAND, ATA_PIO_CMD_READ);
    ata_pio_delay();
    ata_pio_status_t status = ata_pio_get_status();
    ata_pio_read_status_t readstatus;
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

ata_pio_read_status_t ata_pio_readsectors(uint32_t lba, uint8_t sectors_to_read, uint8_t *buf) {
    if (sectors_to_read > DISK_READ_MAX_SECTORS) {
        PANIC("Cannot read more than %d sectors at a time!", DISK_READ_MAX_SECTORS);
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
    ata_pio_read_status_t readstatus;
    ata_pio_status_t status;

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
            buf16[i + (sector * 256)] = ata_pio_read_dataport();
    }

    readstatus.status = status;
    readstatus.data = (uint8_t*)buf;
    return readstatus;
}
    
uint8_t ata_pio_disk_ops_read(uint32_t lba, size_t sector_count, uint8_t *buf) {
    ata_pio_read_status_t status = ata_pio_readsectors(lba, sector_count, buf);
    if (status.status.raw == 0)
        return 0;
    return 1;
}

uint8_t ata_pio_disk_ops_write(uint32_t lba, size_t sector_count, const uint8_t *buf) {
    PANIC("Disk write for ATA PIO is unimplemented!");
}

disk_ops_vtable_t ata_pio_get_disk_ops() {
    disk_ops_vtable_t vtable = {
        .read = ata_pio_disk_ops_read,
        .write = ata_pio_disk_ops_write
    };
    return vtable;
}

/*
void ata_pio_debug_print_status(ata_pio_status_t *status) {
    printf(nameof(ata_pio_status_t) " {\n");
    printf(" %-4s = %d\n", "err", status->flags.err);
    printf(" %-4s = %d\n", "idx", status->flags.idx);
    printf(" %-4s = %d\n", "corr", status->flags.corr);
    printf(" %-4s = %d\n", "drq", status->flags.drq);
    printf(" %-4s = %d\n", "srv", status->flags.srv);
    printf(" %-4s = %d\n", "df", status->flags.df);
    printf(" %-4s = %d\n", "rdy", status->flags.rdy);
    printf(" %-4s = %d\n", "bsy", status->flags.bsy);
    printf("}\n");
}
*/