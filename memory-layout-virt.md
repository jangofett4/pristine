## Pristine OS Virtual Memory Layout

To ease copying of mappings into new processes, everything kernel needs resides in same PML4[511] mapping, which is why we start at 0xFFFF800000000000.

| Start              | End                | Size       | Notes                          |
| ------------------ | ------------------ | ---------- | ------------------------------ |
| 0xFFFF900000000000 | 0xFFFF910000000000 | 1 TiB      | Binning Slab Allocator         |
| 0xFFFF910000000000 | 0xFFFF920000000000 | 1 TiB      | MMIO region                    |
| 0xFFFF920000000000 | 0xFFFF920040000000 | 1 GiB      | Kernel Video Framebuffer       |
| 0xFFFF920040000000 | 0xFFFFFFFF00000000 | ~109 TiB   | Reserved                       |
| 0xFFFFFFFF00000000 | 0xFFFFFFFF40000000 | 1 GiB      | Process Dedicated Kernel Stack |
| 0xFFFFFFFF40000000 | 0xFFFFFFFF80000000 | 1 GiB      | Reserved                       |
| 0xFFFFFFFF80000000 | 0xFFFFFFFFFFFFFFFF | 2 GiB      | Kernel                         |