/*
 * Pristine
 * SPDX-License-Identifier MIT
 */

#pragma once

#include <kernel/kernel.h>
#include <kernel/pmm.h>
#include <common/bootinfo.h>
#include <common/bsfs/bsfs_ops.h>
#include <common/disk.h>
#include <common/serial.h>
#include <drivers/storage/ata/atapio.h>
#include <drivers/video/video.h>
#include <common/idt64.h>
#include <common/gdt.h>

#include <stdint.h>