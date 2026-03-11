ASM      = nasm
ASMFLAGS = -f elf32 -g

C        = clang
COPT	 = -Werror=return-type -Wall -O0 -g -std=c23
CFLAGS32 = -ffreestanding -nostdlib -c -m32 \
           -fno-stack-protector \
		   -DPRINTF_DISABLE_SUPPORT_LONG_LONG \
           -Iinclude -Ilib \
           -MMD -MP -fno-pic -fno-pie -mno-sse -mno-sse2 -mno-mmx \
		   -fno-builtin-memcpy -fno-builtin-memset $(COPT)

CFLAGS64 = -ffreestanding -nostdlib -c -m64 \
           -fno-stack-protector \
		   -DPRINTF_DISABLE_SUPPORT_LONG_LONG \
           -Iinclude -Ilib \
           -MMD -MP -fno-pic -fno-pie -mno-sse -mno-sse2 -mno-mmx \
		   -fno-builtin-memcpy -fno-builtin-memset $(COPT)

LD       = ld.lld

QEMU     = qemu-system-x86_64
QEMUFLAGS = -m 256 -serial stdio -machine pc

# External libraries
LIB32_SRCS = $(shell find lib/lib32 -name "*.c")
LIB32_OBJS = $(patsubst %.c, bin/%.o, $(LIB32_SRCS))

LIB64_SRCS = $(shell find lib/lib64 -name "*.c")
LIB64_OBJS = $(patsubst %.c, bin/%.o, $(LIB64_SRCS))

# Kernel C sources (kernel/, drivers/, lib/)
K_SRCS = $(shell find kernel drivers -name "*.c")
K_OBJS = $(patsubst %.c, bin/%.o, $(K_SRCS)) bin/lib/lib64/printf/printf.o

# Stage2 C sources (boot/*.c) & assembly sources
S2_SRCS		= $(shell find boot/stage2 -name "*.c")
S2_ASMSRCS	= $(shell find boot/stage2 -name "*.asm")

S2_OBJS 	= $(patsubst %.c, bin/%.o, $(S2_SRCS)) bin/lib/lib32/printf/printf.o
S2_ASMOBJS	= $(patsubst %.asm, bin/%.o, $(S2_ASMSRCS))

S1_INCSRCS		= $(shell find boot/stage1 -name "*.inc")
S1_ASMSRCS		= boot/stage1/stage1.asm
S1_ASMOBJS		= $(patsubst %.asm, bin/%.o, $(S1_ASMSRCS))

# Auto-generated header dependencies
-include $(shell find bin -name "*.d")

.PHONY: all clean qemu debug

all: bin/hda.img

# ---- Stage 1 ----
bin/boot/stage1/%.o: boot/stage1/%.asm $(S1_INCSRCS)
	@mkdir -p $(dir $@)
	$(ASM) $(ASMFLAGS) boot/stage1/stage1.asm -o $@

bin/boot/stage1/stage1.elf: $(S1_ASMOBJS)
	$(LD) -T boot/stage1/linker_stage1.ld $^ -o $@

bin/boot/stage1/stage1.bin: bin/boot/stage1/stage1.elf
	@mkdir -p bin
	objcopy -O binary $< $@

# ---- Stage 2 ----
# Stage2 gets its own include path for boot/ headers
bin/boot/stage2/%.o: boot/stage2/%.c
	@mkdir -p $(dir $@)
	$(C) $(CFLAGS32) -Iboot $< -o $@

bin/boot/stage2/%.o: boot/stage2/%.asm
	@mkdir -p $(dir $@)
	$(ASM) -f elf32 $< -o $@

bin/boot/stage2/stage2.elf: $(S2_OBJS) $(S2_ASMOBJS)
	$(LD) -T boot/stage2/linker_stage2.ld $^ -o $@

bin/boot/stage2/stage2.bin: bin/boot/stage2/stage2.elf
	objcopy -O binary $< $@

# ---- Libs ----
bin/lib/lib32/%.o: $(LIB32_SRCS)
	@mkdir -p $(dir $@)
	$(C) $(CFLAGS32) $< -o $@

bin/lib/lib64/%.o: $(LIB64_SRCS)
	@mkdir -p $(dir $@)
	$(C) $(CFLAGS64) $< -o $@

# ---- Kernel ----
bin/kernel/%.o: kernel/%.c
	@mkdir -p $(dir $@)
	$(C) $(CFLAGS64) $< -o $@

bin/drivers/%.o: drivers/%.c
	@mkdir -p $(dir $@)
	$(C) $(CFLAGS64) $< -o $@

bin/kernel.elf: $(K_OBJS)
	$(LD) -T kernel/linker_kernel.ld $^ -o $@

# ---- Final image ----
bin/hda.img: bin/boot/stage1/stage1.bin bin/boot/stage2/stage2.bin bin/kernel.elf
	@mkdir -p bin/hda
	mv bin/kernel.elf root/
	truncate -s 1G $@
	./tools/bsfs/bin/mkfs.bsfs bin/hda.img --label pristine --offset 16
	dd if=bin/boot/stage1/stage1.bin of=$@ bs=512 seek=0 conv=notrunc
	dd if=bin/boot/stage2/stage2.bin of=$@ bs=512 seek=1 conv=notrunc
	./tools/bsfs/bin/bsfs-populate bin/hda.img root --offset 16

# ---- Targets ----
qemu: bin/hda.img
	$(QEMU) $(QEMUFLAGS) -drive file=$(word 1,$^),format=raw

debug: bin/hda.img
	$(QEMU) $(QEMUFLAGS) -drive file=$(word 1,$^),format=raw -s -S

clean:
	rm -rf bin
