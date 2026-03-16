/*
 * Pristine
 * paging: paging related subroutines
 * SPDX-License-Identifier: MIT
 */

#include <common/paging.h>
#include <common/string.h>
#include <stdint.h>

uint64_t *volatile __pg_pml4 = (uint64_t*)PG_PML4_ADDRESS;
uint64_t *volatile __pg_pdpt = (uint64_t*)PG_PDPT_ADDRESS;
uint64_t *volatile __pg_pd   = (uint64_t*)PG_PD_ADDRESS;
uint64_t *volatile __pg_pt0  = (uint64_t*)PG_PT0_ADDRESS;
uint64_t *volatile __pg_pt1  = (uint64_t*)PG_PT1_ADDRESS;