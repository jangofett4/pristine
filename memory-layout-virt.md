## Pristine OS Virtual Memory Layout

Mappings no longer reside at PML[511], it spans multiple entries. This is not much of an issue with fast memcpy to be honest.

| Start              | End                | Size       | Notes                          |
| ------------------ | ------------------ | ---------- | ------------------------------ |
| 0xFFFF900000000000 | 0xFFFF910000000000 | 1 TiB      | Binning Slab Allocator         |
| 0xFFFF910000000000 | 0xFFFF920000000000 | 1 TiB      | MMIO region                    |
| 0xFFFF920000000000 | 0xFFFF920040000000 | 1 GiB      | Kernel Video Framebuffer       |
| 0xFFFF920040000000 | 0xFFFFFF7FC0000000 | ~109 TiB   | Reserved                       |
| 0xFFFFFF7FC0000000 | 0xFFFFFF8000000000 | 1 GiB      | Process Dedicated Kernel Stack |
| 0xFFFFFF8000000000 | 0xFFFFFFFF80000000 | 510 GiB    | Reserved (PML4[511])           |
| 0xFFFFFFFF80000000 | 0xFFFFFFFFFFFFFFFF | 2 GiB      | Kernel                         |