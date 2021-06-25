/*
 * Copyright (c) 2020 Institute of Parallel And Distributed Systems (IPADS), Shanghai Jiao Tong University (SJTU)
 * OS-Lab-2020 (i.e., ChCore) is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *   http://license.coscl.org.cn/MulanPSL
 *   THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 *   PURPOSE.
 *   See the Mulan PSL v1 for more details.
 */

#pragma once

#include <common/vars.h>
#include <common/types.h>
#include <common/mmu.h>

#define PAGE_SIZE (0x1000)

void mm_init();
void set_page_table(paddr_t pgtbl);

static inline bool is_user_addr(vaddr_t vaddr)
{
	return vaddr < KBASE;
}

static inline bool is_user_addr_range(vaddr_t vaddr, size_t len)
{
	return (vaddr + len >= vaddr) && is_user_addr(vaddr + len);
}
