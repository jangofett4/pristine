/*
 * Pristine
 * stage2_bsfs: BSFS related subroutines (file open, read, resolve path etc)
 * SPDX-License-Identifier: MIT
 */

#include "stage2_bsfs.h"
#include "stage2_disk.h"
#include "stage2_string.h"
#include "stage2_memory.h"
#include "lib32/printf/printf.h"

static uint8_t _disk_buf[DISK_READ_MAX_SECTORS * DISK_SECTOR_SIZE];

uint32_t bsfs_resolve_path(const BsfsContext *context, const char* path) {
    // this whole function stinks of not returning a proper error code
    // this is probably fine for stage 2 bootloader, however a better error
    // handling needs to be implemented for actual kernel
    size_t length = strlen(path);
    if (length == 0) {
        return UINT32_MAX;
    }

    if (path[0] != '/') {
        return UINT32_MAX; // we cant support relative path names in stage 2
    }

    char name_tmp[BSFS_MAX_DIRENTNAME] = {};

    // eg, "/fonts/.././kernel.elf"
    uint32_t current_inode_idx = context->header->root_inode;
    char ch = '\0';
    for (size_t i = 1; i < length; i++) {
        memset(name_tmp, 0, BSFS_MAX_DIRENTNAME);
        size_t len = 0;
        do {
            ch = path[i];
            if (ch == '/') break;
            i++;
            if (len >= BSFS_MAX_DIRENTNAME - 1) { // -1 for the null terminator
                return UINT32_MAX;
            }
            name_tmp[len] = ch;
            len++;
        } while (i < length);
        name_tmp[len] = '\0'; // TODO: this is probably wrong by the way, needs testing
        BsfsInode inode;
        if (bsfs_read_inode(context, current_inode_idx, &inode) < 0) {
            return UINT32_MAX;
        }
        int found = 0;
        for (size_t b = 0; !found && b < sizeof(inode.blocks_direct) / sizeof(inode.blocks_direct[0]); b++) {
            uint32_t dirent_block_idx = inode.blocks_direct[b];
            if (dirent_block_idx == 0) continue;
            uint32_t dirent_block_offset = (dirent_block_idx * context->header->block_size);
            uint32_t dirent_sector_lba = dirent_block_offset / DISK_SECTOR_SIZE;

            // TODO: statement below is dangerous. block_size / DISK_SECTOR_SIZE might overshoot our sector budget
            // disk ops WILL panic, we won't write to random addresses, but still, dangerous.
            if (!context->disk_ops->read(dirent_sector_lba, context->header->block_size / DISK_SECTOR_SIZE, _disk_buf)) {
                return UINT32_MAX;
            }
            for (size_t d = 0; d < context->header->block_size / sizeof(BsfsDirent); d++) {
                BsfsDirent dirent = ((BsfsDirent*)_disk_buf)[d];
                if (strcmp(dirent.name, name_tmp) == 0) {
                    found = 1;
                    current_inode_idx = dirent.inode;
                    break;
                }
            }
        }
        if (!found) {
            return UINT32_MAX;
        }
    }

    return current_inode_idx;
}

int bsfs_read_inode(const BsfsContext *context, const uint32_t inode_idx, BsfsInode *inode_out) {
    uint32_t inode_offset =
        (context->header->inode_table_start * context->header->block_size) + (sizeof(BsfsInode) * inode_idx);
    uint32_t inode_sector_lba = inode_offset / DISK_SECTOR_SIZE;
    if (!context->disk_ops->read(inode_sector_lba, 1, _disk_buf)) {
        return BSFS_DISK_READ_ERROR;
    }
    *inode_out = ((BsfsInode*)_disk_buf)[inode_offset % DISK_SECTOR_SIZE / sizeof(BsfsInode)];
    return 0;
}

int bsfs_fopen(const BsfsContext *context, const char* path, BsfsFile *file_out) {
    const uint32_t inode_idx = bsfs_resolve_path(context, path);
    if (inode_idx == UINT32_MAX) {
        return BSFS_FOPEN_NOT_A_FILE; // TODO: yeah its probably not right
    }
    
    if (bsfs_read_inode(context, inode_idx, file_out->inode) < 0) {
        return BSFS_DISK_READ_ERROR;
    }
    if (file_out->inode->type != BSFS_INODE_TYPE_FILE) {
        return BSFS_FOPEN_NOT_A_FILE;
    }
    if (file_out->bufsize < context->header->block_size) {
        return BSFS_FOPEN_BUF_TOO_SMALL;
    }
    if (file_out->bufsize % context->header->block_size != 0) {
        return BSFS_FOPEN_BUF_UNALIGNED;
    }

    file_out->position = 0;
    file_out->eof = 0;

    return 0;
}

#define DIV_CEIL(a, b) (((a) + (b) - 1) / (b))
int bsfs_fread(const BsfsContext *context, void *ptr, uint32_t size, uint32_t count, BsfsFile *file)
{
    const uint32_t start_offset = file->position;
    uint32_t end_offset = start_offset + size * count;
    if (end_offset > file->inode->size)
        end_offset = file->inode->size;
    if ((end_offset - start_offset) > file->bufsize)
        end_offset = start_offset + file->bufsize;
    const uint32_t total_bytes_to_read = end_offset - start_offset; // effectively size * count
    if (total_bytes_to_read > DISK_READ_MAX_SECTORS * DISK_SECTOR_SIZE)
    {
        // overshoots internal maximum disk buffer
        return BSFS_FREAD_SIZE_TOO_BIG;
    }

    // const uint32_t end_offset_local_block_idx = end_offset / context->header->block_size;
    const uint32_t sectors_per_block = context->header->block_size / DISK_SECTOR_SIZE;

    uint8_t *bufptr = file->buf;
    const uint32_t start_block = start_offset / context->header->block_size;
    const uint32_t end_block = DIV_CEIL(end_offset, context->header->block_size);
    for (size_t i = start_block; i < end_block; i++)
    {
        if (i < sizeof(file->inode->blocks_direct) / sizeof(file->inode->blocks_direct[0]))
        {
            const uint32_t lba = file->inode->blocks_direct[i] * context->header->block_size / DISK_SECTOR_SIZE;
            if (!context->disk_ops->read(lba, sectors_per_block, bufptr))
            {
                // either real disk failure or total bytes is too big
                // in this case, a real disk failure
                return BSFS_FREAD_DISK_READ_ERROR;
            }
            bufptr += context->header->block_size;
        }
        else
        {
            // L1, L2 & L3 indirection
            // TODO
            return BSFS_FREAD_FILE_TOO_BIG;
        }
    }
    uint32_t offset_in_buf = start_offset % context->header->block_size;
    memcpy(ptr, file->buf + offset_in_buf, total_bytes_to_read);
    file->position = end_offset;
    file->eof = (file->position >= file->inode->size);
    return total_bytes_to_read;
}
#undef DIV_CEIL

int bsfs_fseeko(const BsfsContext *context, BsfsFile *file, uint64_t offset, int whence) {
    if (whence == BSFS_FSEEKO_CUR) {
        file->position += offset;
    } else if (whence == BSFS_FSEEKO_SET) {
        file->position = offset;
    } else if (whence == BSFS_FSEEKO_END) {
        file->position = file->inode->size - offset;
    }

    if (file->position >= file->inode->size) {
        file->position = file->inode->size;
        file->eof = 1;
    } else {
        file->eof = 0;
    }
    
    return 0;
}

int bsfs_fclose(BsfsFile *file) {
    file->inode = 0;
    file->position = UINT64_MAX;
    return 0;
}
