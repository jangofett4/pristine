/*
 * Pristine
 * stage2_fat16 - fat16 driver for stage 2 bootloader
 * SPDX-License-Identifier: MIT
 */

#include "stage2_fat16.h"
#include "stage2_disk.h"
#include "stage2_common.h"
#include "stage2_memory.h"
#include "lib32/printf/printf.h"

const DiskOpsVtable *_default_ops;
Fat16Header header;
size_t _partition_base;
size_t _fat_start;
size_t _fat2_start;
size_t _root_start;
size_t _root_size;
size_t _data_start;

Fat16Header fat16_init(const DiskOpsVtable *disk_ops, size_t partition_base) {
    _default_ops = disk_ops;
    _partition_base = partition_base;

    disk_ops->read(partition_base, 1, (uint8_t*)&header);

    _fat_start = partition_base + header.bpb.num_reserved_sectors;
    _fat2_start = _fat_start + header.bpb.num_sector_per_fat;
    _root_start = _fat_start + (header.bpb.num_fat * header.bpb.num_sector_per_fat);
    _root_size = (header.bpb.num_root_entries * 32) / header.bpb.num_bytes_per_sector;
    _data_start = _root_start + _root_size;
    return header;
}

Fat16File fat16_find_root_file(char *name, char *ext) {
    Fat16Item item;
    uint8_t found = 0;
    uint8_t buf[512];
    ASSERT(_default_ops->read(_root_start, 1, buf), "Unable to read root directory");
    for (size_t i = 0; i < header.bpb.num_bytes_per_sector / 32; i++) {
        if (buf[i * 32] == 0) break;
        memcpy(&item, buf + (i * 32), 32);
        if (item.attribute.volume_id) continue;
        if (
            item.name[0] == name[0] && 
            item.name[1] == name[1] && 
            item.name[2] == name[2] && 
            item.name[3] == name[3] && 
            item.name[4] == name[4] && 
            item.name[5] == name[5] && 
            item.name[6] == name[6] && 
            item.name[7] == name[7] && 
            item.extension[0] == ext[0] && 
            item.extension[1] == ext[1] && 
            item.extension[2] == ext[2]
        ) {
            found = 1;
            break;
        }
    }
    if (found) {
        Fat16File file;
        fat16_file_open(&item, &file);
        return file;
    } else {
        Fat16File file = {
            .bytes_remaining = 0,
            .current_cluster = 0,
            .current_sector = 0,
            .data = 0,
            .eof = 1,
            .exists = 0
        };
        return file;
    }
}

uint32_t fat16_cluster_to_lba(const Fat16File *file) {
    return _data_start + (file->current_cluster - 2) * header.bpb.num_sector_per_cluster;
}

void fat16_file_open(const Fat16Item *item, Fat16File *file) {
    file->current_cluster = item->cluster_low;
    file->bytes_remaining = item->size;
    file->eof = item->size == 0 ? 1 : 0;
    file->exists = 1;
}

void fat16_file_read(const Fat16Item *item, Fat16File *file, uint8_t *buf) {
    uint32_t lba = fat16_cluster_to_lba(file);
    uint32_t addr;
    for (addr = 0; addr < header.bpb.num_sector_per_cluster; addr++) {
        _default_ops->read(lba, 1, buf + (addr * 512));
    }
}