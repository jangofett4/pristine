/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <common/disk.h>
#include <common/bsfs/bsfs.h>

#define BSFS_FOPEN_FILE_NOTFOUND   -1
#define BSFS_FOPEN_NOT_A_FILE      -2
#define BSFS_DISK_READ_ERROR       -3
#define BSFS_FREAD_FILE_TOO_BIG    -4
#define BSFS_FOPEN_BUF_TOO_SMALL   -5
#define BSFS_FOPEN_BUF_UNALIGNED   -6
#define BSFS_FREAD_DISK_READ_ERROR -7
#define BSFS_FREAD_SIZE_TOO_BIG    -8
#define BSFS_FREAD_BUF_TOO_SMALL   -9

#define BSFS_FSEEKO_SET -1
#define BSFS_FSEEKO_CUR 0
#define BSFS_FSEEKO_END 1

typedef struct {
    const BsfsHeader *header;
    const DiskOpsVtable *disk_ops;
    uint32_t phys_sector_size;
    uint8_t *scratch_buf;
    uint32_t scratch_buf_size;
} BsfsContext;

typedef struct {
    BsfsInode *inode;
    uint8_t *buf;
    uint32_t bufsize;
    uint64_t position;
    int eof;
} BsfsFile;

uint32_t bsfs_resolve_path(const BsfsContext *context, const char* path);
int bsfs_read_inode(const BsfsContext *context, const uint32_t inode_idx, BsfsInode *inode_out);
int bsfs_fopen(const BsfsContext *context, const char* path, BsfsFile *file_out);
int bsfs_fread(const BsfsContext *context, void *ptr, uint32_t size, uint32_t count, BsfsFile *file);
int bsfs_fseeko(const BsfsContext *context, BsfsFile *file, uint64_t offset, int whence);
int bsfs_fclose(BsfsFile *file);