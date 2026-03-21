## Pristine OS Project

Pristine is an OS project mainly developed for learning purposes.  
There is not really an end goal here, currently aiming to run Doom.

### Folder Layout

- `kernel/` - kernel source code, entry point, memory management, interrupt handling
- `boot/` - stage 1 (MBR, 512 bytes) and stage 2 bootloader (ELF loader, paging setup, long mode transition)
- `drivers/` - hardware driver source code (ATA PIO, VESA framebuffer, serial)
- `lib/` - external libraries, split into `lib32/` (stage 2) and `lib64/` (kernel)
- `root/` - root filesystem contents, anything placed here gets copied into the disk image on build
- `tools/` - custom tools for working with BSFS (`mkfs.bsfs`, `bsfs-populate`, `bsfs-extract`)
- `include/` - shared headers between kernel, bootloader, and drivers

### OS Layout

Pristine uses a custom bootloader, custom filesystem (BSFS), and a higher half kernel. Exacty memory layout is given in `memory-layout.md`.

### Architecture

- **Bootloader** - two stage, custom written. Stage 1 fits in 512 bytes (MBR), loads stage 2. Stage 2 implements a BSFS driver, ELF loader, sets up long mode and higher half paging, and jumps to the kernel.
- **Filesystem** - BSFS, a custom inode based filesystem with 4 KiB blocks, CRC32 checksums, direct and L1, L2 and L3 indirect block addressing.
- **Memory** - physical memory managed via a bitmap allocator (PMM). Virtual memory managed via a page table walker (VMM) supporting 4 KiB, 2 MiB (large), and 1 GiB (huge) pages. Kernel lives in higher half, all physical memory directly accessible via HHDM.
- **Interrupts** - IDT with 48 entries (32 CPU exceptions + 16 PIC IRQs), PIC remapped to avoid conflicts with CPU exceptions.

### Requirements

- `make` - makefile support
- `clang` - C compiler
- `nasm` - assembler
- `ld.lld` - linker
- `qemu-system-x86_64` - emulator
- `bear` - for generating `compile_commands.json` (optional, for clangd)
- `libclang_rt` - clang runtime

For `-O2` optimized builds:
```sh
$ ./build.sh
# or 
$ bear --append -- make all
# or without bear
$ make all
```

For debug builds:
```sh
$ bear --append -- make DEBUG=1 all
# or without bear
$ make DEBUG=1 all
```

To launch Qemu with debugging enabled:
```sh
$ ./debug.sh
# or
$ bear --append -- make DEBUG=1 all
$ make debug
# or without bear
$ make DEBUG=1 all debug
```