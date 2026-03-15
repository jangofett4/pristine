/*
 * Pristine
 * stage2_paging: paging related subroutines
 * SPDX-License-Identifier: MIT
 */

#include "include/stage2_paging.h"

#include <common/string.h>
#include <stdint.h>


#define PG_PML4_ADDRESS 0x1000
#define PG_PDPT_ADDRESS 0x2000
#define PG_PD_ADDRESS   0x3000
#define PG_PT0_ADDRESS  0x4000
#define PG_PT1_ADDRESS  0x5000

uint64_t *volatile __pg_pml4 = (uint64_t*)PG_PML4_ADDRESS;
uint64_t *volatile __pg_pdpt = (uint64_t*)PG_PDPT_ADDRESS;
uint64_t *volatile __pg_pd   = (uint64_t*)PG_PD_ADDRESS;
uint64_t *volatile __pg_pt0  = (uint64_t*)PG_PT0_ADDRESS;
uint64_t *volatile __pg_pt1  = (uint64_t*)PG_PT1_ADDRESS;