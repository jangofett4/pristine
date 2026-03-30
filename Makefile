export REPO_ROOT = $(shell pwd)

ASM      = nasm
ASMFLAGS = -f elf32 -g

CGUEST32       = ${CROSSGCC32}gcc
CGUEST64       = ${CROSSGCC64}gcc
LDGUEST32      = ${CROSSGCC32}ld
LDGUEST64      = ${CROSSGCC64}ld
OBJCOPYGUEST32 = ${CROSSBINUTILS32}objcopy
OBJCOPYGUEST64 = ${CROSSBINUTILS64}objcopy

export CGUEST64
export LDGUEST64

LDHOST   = ld
CHOST    = gcc

LDFLAGS  = -nostdlib -nostartfiles -nodefaultlibs -lgcc

COPT	 = -Werror=return-type -Wall -std=c23
CFLAGS32 = -ffreestanding -c -m32 \
           -fno-stack-protector -mno-red-zone \
           -Iinclude -Ilib -fno-omit-frame-pointer \
		   -fno-asynchronous-unwind-tables \
           -MMD -MP -fno-pic -fno-pie -mno-sse -mno-sse2 -mno-mmx \
		   -fno-builtin-memcpy -fno-builtin-memset $(COPT)

CFLAGS64 = -ffreestanding -c -m64 \
           -fno-stack-protector -mno-red-zone \
           -Iboot/stage2 -Iinclude -Ilib -fno-omit-frame-pointer \
		   -fno-asynchronous-unwind-tables \
           -MMD -MP -fno-pic -fno-pie -mno-mmx \
		   -fno-builtin-memcpy -fno-builtin-memset $(COPT) -mcmodel=kernel


QEMU      = qemu-system-x86_64
QEMUFLAGS = -m 256 -serial stdio -machine pc

# External libraries
LIB32_SRCS = $(shell find lib/lib32 -name "*.c")
LIB32_OBJS = $(patsubst %.c, bin/%.o, $(LIB32_SRCS))

LIB64_SRCS = $(shell find lib/lib64 -name "*.c")
LIB64_OBJS = $(patsubst %.c, bin/%.o, $(LIB64_SRCS))

# Kernel C sources (kernel/, drivers/, lib/)
K_SRCS     = $(shell find kernel drivers -name "*.c")
K_ASMSRCS  = $(shell find kernel drivers -name "*.asm")

K_OBJS     = $(patsubst %.c, bin/%.o, $(K_SRCS))
K_ASMOBJS  = $(patsubst %.asm, bin/%.o, $(K_ASMSRCS))

K_BUILTINS = $(LIBGCC64_PATH)

# Stage2 C sources (boot/*.c) & assembly sources
S2_SRCS		= $(shell find boot/stage2 -name "*.c")
S2_ASMSRCS	= $(shell find boot/stage2 -name "*.asm")

S2_OBJS 	= $(patsubst %.c, bin/%.o, $(S2_SRCS)) bin/lib/lib32/printf/printf.o
S2_ASMOBJS	= $(patsubst %.asm, bin/%.o, $(S2_ASMSRCS))

S2_BUILTINS = $(LIBGCC32_PATH)

S1_INCSRCS		= $(shell find boot/stage1 -name "*.inc")
S1_ASMSRCS		= boot/stage1/stage1.asm
S1_ASMOBJS		= $(patsubst %.asm, bin/%.o, $(S1_ASMSRCS))

# BSFS
BSFS_MKFS_SRCS     = tools/bsfs/mkfs.bsfs.c
BSFS_POPULATE_SRCS = tools/bsfs/bsfs-populate.c tools/bsfs/bsfs.c
BSFS_EXTRACT_SRCS  = tools/bsfs/bsfs-extract.c tools/bsfs/bsfs.c

CFLAGS_BSFS = -Iinclude/common/bsfs -O2 -std=c11 -DBSFS_DEBUG=1 -DBSFS_STDLIB_EXISTS=1 -g

# Apps

APPS_RAW = libc hello
APPS     = $(patsubst %, userspace/%, $(APPS_RAW))

ifdef DEBUG
	COPT += -DPRISTINE_DEBUG -O0 -g 
else
	COPT += -O2
endif

# Auto-generated header dependencies
-include $(shell find bin -name "*.d" 2>/dev/null)

.PHONY: all $(APPS) clean qemu debug

all: bin/boot/stage1/stage1.bin bin/boot/stage2/stage2.bin bin/kernel.elf bin/mkfs.bsfs bin/bsfs-populate bin/bsfs-extract $(APPS)

# ---- Stage 1 ----
bin/boot/stage1/%.o: boot/stage1/%.asm $(S1_INCSRCS)
	@mkdir -p $(dir $@)
	$(ASM) $(ASMFLAGS) boot/stage1/stage1.asm -o $@

bin/boot/stage1/stage1.elf: $(S1_ASMOBJS)
	$(CGUEST32) -T boot/stage1/linker_stage1.ld $^ $(LDFLAGS) -o $@

bin/boot/stage1/stage1.bin: bin/boot/stage1/stage1.elf
	@mkdir -p bin
	$(OBJCOPYGUEST32) -O binary $< $@

# ---- Stage 2 ----
# Stage2 gets its own include path for boot/ headers
bin/boot/stage2/%.o: boot/stage2/%.c
	@mkdir -p $(dir $@)
	$(CGUEST32) $(CFLAGS32) -Iboot $< -o $@

bin/boot/stage2/%.o: boot/stage2/%.asm
	@mkdir -p $(dir $@)
	$(ASM) -f elf32 $< -o $@

bin/boot/stage2/stage2.elf: $(S2_OBJS) $(S2_ASMOBJS) 
	$(CGUEST32) -T boot/stage2/linker_stage2.ld $^ $(LDFLAGS) -o $@ 

bin/boot/stage2/stage2.bin: bin/boot/stage2/stage2.elf
	$(OBJCOPYGUEST32) -O binary $< $@

# ---- Libs ----
bin/lib/lib32/%.o: $(LIB32_SRCS)
	@mkdir -p $(dir $@)
	$(CGUEST32) $(CFLAGS32) $< -o $@

bin/lib/lib64/%.o: $(LIB64_SRCS)
	@mkdir -p $(dir $@)
	$(CGUEST64) $(CFLAGS64) $< -o $@

# ---- Kernel ----
bin/kernel/%.o: kernel/%.c
	@mkdir -p $(dir $@)
	$(CGUEST64) $(CFLAGS64) $< -o $@

bin/drivers/%.o: drivers/%.c
	@mkdir -p $(dir $@)
	$(CGUEST64) $(CFLAGS64) $< -o $@

bin/kernel/%.o: kernel/%.asm
	@mkdir -p $(dir $@)
	$(ASM) -f elf64 -g $< -o $@

bin/drivers/%.o: drivers/%.asm
	@mkdir -p $(dir $@)
	$(ASM) -f elf64 -g $< -o $@

bin/kernel.elf: $(K_OBJS) $(K_ASMOBJS)
	$(CGUEST64) -T kernel/linker_kernel.ld $^ $(LDFLAGS) -o $@

# ---- BSFS ----
bin/mkfs.bsfs: $(BSFS_MKFS_SRCS)
	@mkdir -p $(dir $@)
	$(CHOST) $(CFLAGS_BSFS) $^ -o $@

bin/bsfs-populate: $(BSFS_POPULATE_SRCS)
	@mkdir -p $(dir $@)
	$(CHOST) $(CFLAGS_BSFS) $^ -o $@

bin/bsfs-extract: $(BSFS_EXTRACT_SRCS)
	@mkdir -p $(dir $@)
	$(CHOST) $(CFLAGS_BSFS) $^ -o $@

# ---- Apps ----
$(APPS):
	$(MAKE) -C $@

# ---- Final image ----
bin/hda.img: bin/boot/stage1/stage1.bin bin/boot/stage2/stage2.bin bin/kernel.elf bin/mkfs.bsfs bin/bsfs-populate bin/bsfs-extract
	@mkdir -p bin/hda
	mv bin/kernel.elf root/
	truncate -s 1G $@
	./bin/mkfs.bsfs bin/hda.img --label pristine --offset 16
	dd if=bin/boot/stage1/stage1.bin of=$@ bs=512 seek=0 conv=notrunc
	dd if=bin/boot/stage2/stage2.bin of=$@ bs=512 seek=1 conv=notrunc
	./bin/bsfs-populate bin/hda.img root --offset 16

# ---- Targets ----
qemu: bin/hda.img
	$(QEMU) $(QEMUFLAGS) -drive file=$(word 1,$^),format=raw

debug: COPT += -DPRISTINE_DEBUG -O0 -g 
debug: bin/hda.img
	$(QEMU) $(QEMUFLAGS) -drive file=$(word 1,$^),format=raw -s -S

clean:
	rm -rf bin
	for app in $(APPS); do $(MAKE) -C $$app clean; done