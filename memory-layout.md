## Pristine OS Memory Layout

As I keep working on this project, I keep overwriting data I originally placed on a fixed address.

This file (hopefully) will make me more mindful about what I put where. Every address is physical.

| Start          | End            | Size           | Description                        |
| -------------- | -------------- | -------------- | ---------------------------------- |
| 0x0000         | 0x1000         | 4 KiB          | Reserved, BIOS, IVT, Legacy Code   |
| 0x1000         | 0x6000         | 5 KiB          | Free                               |
| 0x6000         | 0x7000         | 4 KiB          | E820h Memory Map (20 byte entries) |
| 0x7000         | 0x7100         | 256 bytes      | VESA VBE Info Buffer               |
| 0x7100         | 0x7200         | 256 bytes      | VESA VBE Mode Info Buffer          |
| 0x7300         | 0x7400         | 256 bytes      | GDT, 32 entries supported          |
| 0x7400         | 0x7500         | 256 bytes      | TSS, 2 entries supported           |
| 0x7C00         | 0x7E00         | 512 bytes      | Stage 1 Bootloader                 |
| 0x7E00         | 0x17C00        | 65024 bytes    | Stage 2 Bootloader (127 sectors)   |
| 0x17C00        | 0x100000       | 929 KiB        | Reserved (shouldn't be used)       |
| 0x100000       | 0x1C0000       | 768 KiB        | Memory Bitmap                      |
| 0x1C0000       | 0x1C1000       | 4 KiB          | PML4 (512 entries available)       |
| 0x1C1000       | 0x1C2000       | 4 KiB          | PDPT Kernel                        |
| 0x1C2000       | 0x1C3000       | 4 KiB          | PDPT Higher Half Direct Mapping    |
| 0x1C3000       | 0x1C4000       | 4 KiB          | PDPT Stage 2 Direct Mapping        |
| 0x1C4000       | 0x1C5000       | 4 KiB          | PD Kernel                          |
| 0x1C5000       | 0x1C6000       | 4 KiB          | PD Stage 2 Direct Mapping          |
| 0x1C6000       | 0x1F7000       | 196 KiB        | Free                               |
| 0x1F7000       | 0x3F7000       | 2048 KiB       | Free                               | 
| 0x3F7000       | 0x1F8000       | 4 KiB          | IST1 Guard Page                    |
| 0x3F8000       | 0x1F8FF8       | 4088 bytes     | IST1                               |
| 0x3F9000       | 0x1FA000       | 4 KiB          | RSP0 Guard Page                    |
| 0x3FA000       | 0x1FAFF8       | 4088 bytes     | RSP0                               |
| 0x3FB000       | 0x1FC000       | 4 KiB          | Kernel Stack Guard Page            |
| 0x3FC000       | 0x1FFFF8       | 16376 bytes    | Kernel Stack                       |
| 0x400000       | 0x600000       | 2 MiB          | Kernel                             |