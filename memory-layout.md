## Pristine OS Memory Layout

As I keep working on this project, I keep overwriting data I originally placed on a fixed address.

This file (hopefully) will make me more mindful about what I put where.

| Start          | End            | Size           | Description                              |
| -------------- | -------------- | -------------- | ---------------------------------------- |
| 0x00000000     | 0x00001000     | 4 KiB          | Reserved, BIOS, IVT, Legacy Code         |
| 0x00001000     | 0x00006000     | 20 KiB         | Free                                     |
| 0x00006000     | 0x00006002     | 2 bytes        | E820h Memory Map Entry Count             |
| 0x00006002     | 0x00007000     | ~4 KiB         | E820h Memory Map (20 byte entries)       |
| 0x00007000     | 0x00007100     | 256 bytes      | VESA VBE Info Buffer                     |
| 0x00007100     | 0x00007200     | 256 bytes      | VESA VBE Mode Info Buffer                |
| 0x00007300     | 0x00007400     | 256 bytes      | GDT (stage 2 only, 32 entries)           |
| 0x00007C00     | 0x00007E00     | 512 bytes      | Stage 1 Bootloader                       |
| 0x00007E00     | 0x00017C00     | ~64 KiB        | Stage 2 Bootloader (127 sectors)         |
| 0x00017C00     | 0x00100000     | ~929 KiB       | Reserved (BIOS ROM, EBDA, do not use)    |
| 0x00100000     | 0x001C0000     | 768 KiB        | Physical Memory Bitmap (24 GiB coverage) |
| 0x001C0000     | 0x001C1000     | 4 KiB          | PML4 (1 table, 512 entries)              |
| 0x001C1000     | 0x001C4000     | 12 KiB         | PDPTs (3 tables: kernel, HHDM, stage 2)  |
| 0x001C4000     | 0x001C6000     | 8 KiB          | PDs (2 tables: kernel, stage 2)          |
| 0x001C6000     | 0x001C8000     | 8 KiB          | PTs (2 tables: 0-2MiB, 2-4MiB)           |
| 0x001C8000     | 0x00400000     | ~2.2 MiB       | Free                                     |
| 0x00400000     | 0x00600000     | 2 MiB          | Kernel ELF (physical load address)       |

### Notes

- Stage 2 GDT at 0x7300 is copied and relocated by kernel early init via PMM
- TSS is not set up by stage 2, kernel allocates and initializes fresh via PMM
- Page tables are stage 2 bootstrap only, VMM replaces them
- Kernel runs at virtual 0xFFFFFFFF80400000, physical 0x400000
- Everything above 0x600000 is managed by PMM based on E820 map