/*
 * Pristine
 * SPDX-License-Identifier: MIT
 */

#pragma once

#include <stdint.h>
#include "stage2_disk.h"

typedef struct {
    uint8_t instr[3];               // Assembly instructions (likely jmp 3c, nop)
    char oem[8];                    // OEM identifier
    uint16_t num_bytes_per_sector;  // Bytes per sector
    uint8_t num_sector_per_cluster; // Sectors per cluster
    uint16_t num_reserved_sectors;  // Number of reserved sectors
    uint8_t num_fat;                // Number of FATs
    uint16_t num_root_entries;      // Number of root directory entries
    uint16_t num_sectors;           // Number of sectors
    uint8_t media_descriptor;       // Media descriptor (?)
    uint16_t num_sector_per_fat;    // Number of sectors per FAT
    uint16_t num_sector_per_track;  // Number of sectors per track
    uint16_t num_sides;             // Number of sides
    uint32_t num_hidden_sectors;    // Number of hidden sectors
    uint32_t num_sectors_large;     // Large sector count
} __attribute__((packed)) Fat16Bpb;

typedef struct {
    char num_drive;
    char flags;
    uint8_t signature;
    uint8_t serial[4];
    char label[11];
    char sysident[8];
    uint8_t bootcode[448];
    uint8_t bootsignature[2];
} __attribute__((packed)) Fat16Ebpb;

typedef struct {
    Fat16Bpb bpb;
    Fat16Ebpb ebpb;
} __attribute__((packed)) Fat16Header;

typedef struct {
    char     name[8];
    char     extension[3];
    struct {
        uint8_t readonly  : 1;
        uint8_t hidden    : 1;
        uint8_t system    : 1;
        uint8_t volume_id : 1;
        uint8_t directory : 1;
        uint8_t archive   : 1;
        uint8_t device    : 1;
        uint8_t reserved  : 1;
    } __attribute__((packed)) attribute;
    uint8_t  nt_reserved;       // Windows NT reserved, always 0
    uint8_t  creation_tenths;   // Creation time tenths of second
    uint16_t creation_time;     // Creation time
    uint16_t creation_date;     // Creation date
    uint16_t access_date;       // Last access date
    uint16_t cluster_high;      // High 16 bits of cluster (always 0 in FAT16)
    uint16_t modified_time;     // Last modified time
    uint16_t modified_date;     // Last modified date
    uint16_t cluster_low;       // Low 16 bits of start cluster
    uint32_t size;
} __attribute__((packed)) Fat16Item;

typedef struct {
    uint32_t current_cluster;
    uint32_t current_sector;
    uint32_t bytes_remaining;
    uint8_t  *data;
    uint8_t  eof;
    uint8_t  exists;
} Fat16File;

Fat16Header fat16_init(const DiskOpsVtable *disk_ops, size_t partition_base);
void fat16_file_open(const Fat16Item *item, Fat16File *file);
void fat16_file_read(const Fat16Item *item, Fat16File *file, uint8_t *buf);
Fat16File fat16_find_root_file(char *name, char *ext);