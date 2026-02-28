## BSFS

"BaSic File System", "Basic aSs File System", "Butt Simple File System", "Boot Sector File System", "BullShit File System", ...

Whatever you want to call it, is a basic but capable filesystem, made mainly for learning purposes.

### Filesystem Layout

For the remainder of this documentation, we will use sectors & blocks, a sector is a (generally) 512 bytes of data, and a block is (by default) 4096 bytes. 8 sectors make up a single block in BSFS.

```
       Block 1                         Block 2
      |-------------------------------|--------...
      v                               v
Disk: |---|---|---|---|---|---|---|---|---|---|---|---|---|---|
      ^   ^   ^                                               ^
      |   |   |                                               |
      Sector 0|                                               Sector N
          |   |
          Sector 1
              |
              Sector 3
```

BSFS both supports starting from first sector and can contain boot code, it also supports starting from an offset.  

Default BSFS layout with bootcode (disk size 128MiB):
| Structure | Offset (Bytes) | Size (Sectors) | Size (Bytes) | Notes |
|---|---|---|---|---|
| BSFS Header | 0 | 1 | 512 | |
| Empty space | 512 | 63 | 32,256 | Space reserved for stage 2 bootloader |
| Block Bitmap | 32,768 | 8 | 4,096 | 1 bit per block |
| Inode Bitmap | 36,864 | 8 | 4,096 | 1 bit per inode, padded to 1 block |
| Inode Table | 40,960 | 2,048 | 1,048,576 | 8,192 inodes × 128 bytes |
| Data | 1,089,536 | 259,632 | 133,128,192 | 32,502 data blocks |

Default BSFS layout without bootcode (disk size 128MiB):
| Structure | Offset (Bytes) | Size (Sectors) | Size (Bytes) | Notes |
|---|---|---|---|---|
| Boot Code | 0 | 64 | 32,768 | BYOK bootloader |
| BSFS Header | 32,768 | 1 | 512 | |
| Padding | 33,280 | 7 | 3,584 | Align to 4K block boundary |
| Block Bitmap | 36,864 | 8 | 4,096 | 1 bit per block |
| Inode Bitmap | 40,960 | 8 | 4,096 | 1 bit per inode, padded to 1 block |
| Inode Table | 45,056 | 2,048 | 1,048,576 | 8,192 inodes × 128 bytes |
| Data | 1,093,632 | 259,024 | 133,124,096 | 32,501 data blocks |

### Typical File Lookup
Lets assume the following filesystem layout
```
/
/boot
/boot/kernel.elf
/boot/config.txt
/bin
/usr
```

To find `kernel.elf` we start by reading

TODO: Actually explain how to...