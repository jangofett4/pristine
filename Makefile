ASM      = nasm
ASMFLAGS = -f elf32

C        = clang
COPT	 = 
CFLAGS   = -ffreestanding -nostdlib -c -m32 -g -std=c23 \
           -fno-stack-protector \
           -DPRINTF_DISABLE_SUPPORT_LONG_LONG \
           -Iinclude -Ilib \
           -MMD -MP -fno-pic -fno-pie -mno-sse -mno-sse2 -mno-mmx \
		   -fno-builtin-memcpy -fno-builtin-memset $(COPT)

S2CFLAGS = -ffreestanding -nostdlib -c -m32 -g -std=c23 \
           -fno-stack-protector \
           -Iinclude -Ilib \
           -MMD -MP -fno-pic -fno-pie -mno-sse -mno-sse2 -mno-mmx \
		   -fno-builtin-memcpy -fno-builtin-memset $(COPT)

LD       = ld.lld

QEMU     = qemu-system-i386
QEMUFLAGS = -m 128 -machine pc -serial stdio

# Kernel C sources (kernel/, drivers/, lib/)
K_SRCS = $(shell find kernel drivers lib -name "*.c")
K_OBJS = $(patsubst %.c, bin/%.o, $(K_SRCS))

# Stage2 C sources (boot/*.c)
S2_SRCS		= $(shell find boot/stage2 -name "*.c")
S2_ASMSRCS	= $(shell find boot/stage2 -name "*.asm")
S2_OBJS 	= $(patsubst %.c, bin/%.o, $(S2_SRCS))
S2_ASMOBJS	= $(patsubst %.asm, bin/%.o, $(S2_ASMSRCS)) bin/lib/printf.o

S1_ASMSRCS		= boot/stage1/stage1.asm
S1_ASMOBJS		= $(patsubst %.asm, bin/%.o, $(S1_ASMSRCS))

# Auto-generated header dependencies
-include $(shell find bin -name "*.d")

.PHONY: all clean qemu debug

all: bin/fda.raw

# ---- Stage 1 ----
bin/boot/stage1/%.o: boot/stage1/%.asm
	@mkdir -p $(dir $@)
	$(ASM) $(ASMFLAGS) $< -o $@

bin/boot/stage1/stage1.elf: $(S1_ASMOBJS)
	$(LD) -T boot/stage1/linker_stage1.ld $^ -o $@

bin/boot/stage1/stage1.bin: bin/boot/stage1/stage1.elf
	@mkdir -p bin
	objcopy -O binary $< $@

# ---- Stage 2 ----
# Stage2 gets its own include path for boot/ headers
bin/boot/stage2/%.o: boot/stage2/%.c
	@mkdir -p $(dir $@)
	$(C) $(S2CFLAGS) -Iboot $< -o $@

bin/boot/stage2/%.o: boot/stage2/%.asm
	@mkdir -p $(dir $@)
	$(ASM) -f elf32 $< -o $@

bin/boot/stage2/stage2.elf: $(S2_OBJS) $(S2_ASMOBJS)
	$(LD) -T boot/stage2/linker_stage2.ld $^ -o $@

bin/boot/stage2/stage2.bin: bin/boot/stage2/stage2.elf
	objcopy -O binary $< $@

# ---- Kernel ----
bin/%.o: %.c
	@mkdir -p $(dir $@)
	$(C) $(CFLAGS) $< -o $@

bin/kernel.elf: $(K_OBJS)
	$(LD) -T kernel/linker_kernel.ld $^ -o $@

# ---- Final image ----
bin/hda.img: bin/boot/stage1/stage1.bin bin/boot/stage2/stage2.bin bin/kernel.elf
	@mkdir -p bin/hda
	mv bin/kernel.elf bin/hda/
	truncate -s 32M $@
	dd if=bin/boot/stage1/stage1.bin of=$@ bs=512 seek=0 conv=notrunc
	dd if=bin/boot/stage2/stage2.bin of=$@ bs=512 seek=1 conv=notrunc
	@echo 'drive i: file="$(PWD)/bin/hda.img" offset=32768' > mtools.cfg
	MTOOLSRC=mtools.cfg mformat -v "PRISTINE" i:
	MTOOLSRC=mtools.cfg mcopy -o -s bin/hda/* i:

# ---- Targets ----
qemu: bin/hda.img
	$(QEMU) $(QEMUFLAGS) -drive file=$(word 1,$^),format=raw

debug: bin/hda.img
	$(QEMU) $(QEMUFLAGS) -drive file=$(word 1,$^),format=raw -s -S

clean:
	rm -rf bin