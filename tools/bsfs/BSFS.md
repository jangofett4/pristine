# BSFS — BaSic File System

BSFS is a simple inode-based filesystem designed for learning purposes, inspired by ext2. It is used as the primary filesystem for the Pristine OS project.

---

## Limits

| Property | Value |
|---|---|
| Default block size | 4096 bytes |
| Min block size | 1024 bytes |
| Max block size | 16384 bytes |
| Max volume size | 16 TiB (with 4096 byte blocks) |
| Max file size (direct only) | 40 KiB (10 × block size) |
| Max file size (L1 indirect) | ~4 MiB |
| Max file size (L2 indirect) | ~4 GiB |
| Max file size (L3 indirect) | ~4 TiB |
| Max filename length | 123 characters (+ null terminator) |
| Max dirents per block | 32 (4096 / 128) |
| Inode count | total_blocks / 4 |

---

## Disk Layout

BSFS supports two layouts depending on whether the filesystem starts at sector 0 or at an offset.

**With bootcode (offset = 0):**

| Structure | Offset (Bytes) | Size | Notes |
|---|---|---|---|
| BSFS Header | 0 | 1 sector | Contains embedded stage 1 + jump to stage 2 |
| Stage 2 | 512 | 63 sectors | Space for stage 2 bootloader |
| Block Bitmap | 32768 | 1 block | 1 bit per block |
| Inode Bitmap | 36864 | 1 block | 1 bit per inode |
| Inode Table | 40960 | inode_count × 128 bytes | |
| Data | variable | remaining blocks | |

**With external bootloader (offset = 8 in this case):**

| Structure | Offset (Bytes) | Size | Notes |
|---|---|---|---|
| Boot Code | 0 | 64 sectors | Bring your own bootloader |
| BSFS Header | 32768 | 1 sector | |
| Padding | 33280 | 7 sectors | Align to block boundary |
| Block Bitmap | 36864 | 1 block | 1 bit per block |
| Inode Bitmap | 40960 | 1 block | 1 bit per inode |
| Inode Table | 45056 | inode_count × 128 bytes | |
| Data | variable | remaining blocks | |

---

## Structures

### `bsfs_header_t` (512 bytes, sector 0 or at offset)

| Field | Type | Description |
|---|---|---|
| bootjmp | uint8_t[3] | x86 short jump over metadata (`0xEB, offset, 0x90`) |
| magic | uint32_t | Must be `0x42534653` ("BSFS") |
| version | uint32_t | Packed version: `major.minor.patch.rev` |
| block_size | uint32_t | Block size in bytes |
| total_blocks | uint32_t | Total number of blocks in the volume |
| inode_count | uint32_t | Total number of inodes |
| free_blocks | uint32_t | Number of free blocks |
| free_inodes | uint32_t | Number of free inodes |
| block_bitmap_start | uint32_t | Block bitmap start, in blocks |
| block_bitmap_blocks | uint32_t | Block bitmap size, in blocks |
| megablock_bitmap_start | uint32_t | Megablock bitmap start, in blocks |
| megablock_bitmap_blocks | uint32_t | Megablock bitmap size, in blocks |
| megablock_bitmap_size | uint32_t | Megablock bitmap size, in bytes |
| inode_bitmap_start | uint32_t | Inode bitmap start, in blocks |
| inode_bitmap_blocks | uint32_t | Inode bitmap size, in blocks |
| inode_table_start | uint32_t | Inode table start, in blocks |
| inode_table_blocks | uint32_t | Inode table size, in blocks |
| data_start | uint32_t | Data region start, in blocks |
| root_inode | uint32_t | Index of root directory inode |
| partition_lba | uint64_t | Partition offset in blocks |
| label | uint8_t[32] | Volume label, null terminated |
| bootcode | uint8_t[395] | Stage 1 bootcode (offset=0 layout only) |
| bootcode_magic | uint8_t[2] | Boot signature `0x55, 0xAA` |

### `bsfs_inode_t` (128 bytes)

| Field | Type | Description |
|---|---|---|
| type | uint8_t | `FREE`, `FILE`, `DIRECTORY`, `SYMLINK` |
| permissions | uint16_t | `EXEC(0x01)`, `WRITE(0x02)`, `READ(0x04)` |
| uid | uint32_t | Owner user ID |
| gid | uint32_t | Owner group ID |
| size | uint64_t | Size in bytes (files) or dirent count × 128 (dirs) |
| blocks | uint32_t | Number of allocated blocks |
| created | uint64_t | Creation timestamp (Unix epoch) |
| modified | uint64_t | Last modification timestamp (Unix epoch) |
| accessed | uint64_t | Last access timestamp (Unix epoch) |
| link_count | uint32_t | Hard link count |
| blocks_direct[10] | uint32_t | Direct block pointers (~40 KiB) |
| blocks_l1indirect | uint32_t | L1 indirect block pointer (~4 MiB) |
| blocks_l2indirect | uint32_t | L2 indirect block pointer (~4 GiB) |
| blocks_l3indirect | uint32_t | L3 indirect block pointer (~4 TiB) |
| reserved | uint8_t[25] | Reserved, must be zero |

### `bsfs_dirent_t` (128 bytes)

| Field | Type | Description |
|---|---|---|
| inode | uint32_t | Inode index. `0` = free slot |
| name | uint8_t[124] | Filename, null terminated. Max 123 characters |

---

## Inode Types

| Constant | Value | Description |
|---|---|---|
| `BSFS_INODE_TYPE_FREE` | 0 | Free/unallocated inode |
| `BSFS_INODE_TYPE_FILE` | 1 | Regular file |
| `BSFS_INODE_TYPE_DIRECTORY` | 2 | Directory |
| `BSFS_INODE_TYPE_SYMLINK` | 3 | Symbolic link (reserved) |

## Permission Flags

| Constant | Value | Description |
|---|---|---|
| `BSFS_INODE_PERM_EXEC` | 0x01 | Execute |
| `BSFS_INODE_PERM_WRITE` | 0x02 | Write |
| `BSFS_INODE_PERM_READ` | 0x04 | Read |

---

## Tools

**`mkfs.bsfs`** — formats a raw disk image with a BSFS filesystem. Writes the header and all metadata regions. Supports configurable block size and partition offset.

**`bsfs-populate`** — walks a host directory tree and writes its contents into an existing BSFS image, allocating inodes, bitmaps and data blocks as needed. *(rewrite in progress)*

**`bsfs-extract`** — reads an existing BSFS image and extracts its contents to a host directory. *(in progress)*
