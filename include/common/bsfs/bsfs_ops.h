/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <common/disk.h>
#include <common/bsfs/bsfs.h>

#define BSFS_ERRORS(X)\
    X(BSFS_FOPEN_FILE_NOTFOUND,  -1,  "file not found")   \
    X(BSFS_FOPEN_NOT_A_FILE,     -2,  "inode not a file") \
    X(BSFS_DISK_READ_ERROR,      -3,  "disk read error")  \
    X(BSFS_FREAD_FILE_TOO_BIG,   -4,  "file too big")     \
    X(BSFS_FOPEN_BUF_TOO_SMALL,  -5,  "buffer too small") \
    X(BSFS_FOPEN_BUF_UNALIGNED,  -6,  "unaligned buffer") \
    X(BSFS_FREAD_DISK_READ_ERROR,-7,  "disk read error")  \
    X(BSFS_FREAD_SIZE_TOO_BIG,   -8,  "file too big")     \
    X(BSFS_FREAD_BUF_TOO_SMALL,  -9,  "buffer too small") \
    X(BSFS_CHECKSUM_FAILED,      -10, "checksum failed")

#define BSFS_DEFINE(name, code, msg) static const int name = code;
BSFS_ERRORS(BSFS_DEFINE);

#undef BSFS_DEFINE

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
    uint32_t cached_block;
    
    uint32_t *l1_table;
    uint32_t cached_l1;

    uint32_t *l2_table;
    uint32_t cached_l2;

    uint32_t *l3_table;
    uint32_t cached_l3;
    int eof;
} BsfsFile;

uint32_t bsfs_resolve_path(const BsfsContext *context, const char* path);
int bsfs_read_inode(const BsfsContext *context, const uint32_t inode_idx, BsfsInode *inode_out);
int bsfs_fopen(const BsfsContext *context, const char* path, BsfsFile *file_out);
int bsfs_fread(const BsfsContext *context, void *ptr, uint32_t size, uint32_t count, BsfsFile *file);
int bsfs_fseeko(const BsfsContext *context, BsfsFile *file, uint64_t offset, int whence);
int bsfs_fclose(BsfsFile *file);
const char *bsfs_strerror(int err);

// Verifies checksum of the given file. Returns negative value on error or failed checksum. Caller should use bsfs_strerror to get error string.
// Caller should bsfs_fseeko back to beginning if it wants to read the file, as this function reads the whole file once.
int bsfs_verify_checksum(const BsfsContext *context, BsfsFile *file, uint8_t *buf, size_t bufsize);