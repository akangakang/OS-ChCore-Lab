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

#include <common/types.h>
#include <common/macro.h>
#include <common/util.h>
#include <common/errno.h>

#include "slab.h"
#include "buddy.h"

#define _SIZE (1UL << SLAB_MAX_ORDER)

u64 size_to_page_order(u64 size)
{
	u64 order;
	u64 pg_num;
	u64 tmp;

	order = 0;
	pg_num = ROUND_UP(size, BUDDY_PAGE_SIZE) / BUDDY_PAGE_SIZE;
	tmp = pg_num;

	while (tmp > 1) {
		tmp >>= 1;
		order += 1;
	}

	if (pg_num > (1 << order))
		order += 1;

	return order;
}

void *kmalloc(size_t size)
{
	u64 order;
	struct page *p_page;

	if (size <= _SIZE) {
		return alloc_in_slab(size);
	}

	if (size <= BUDDY_PAGE_SIZE)
		order = 0;
	else
		order = size_to_page_order(size);

	p_page = buddy_get_pages(&global_mem, order);
	return page_to_virt(&global_mem, p_page);
}

void *kzalloc(size_t size)
{
	void *ptr;

	ptr = kmalloc(size);

	/* lack of memory */
	if (ptr == NULL)
		return NULL;

	memset(ptr, 0, size);
	return ptr;
}

void kfree(void *ptr)
{
	struct page *p_page;

	p_page = virt_to_page(&global_mem, ptr);
	if (p_page && p_page->slab)
		free_in_slab(ptr);
	else
		buddy_free_pages(&global_mem, p_page);
}

void *get_pages(int order)
{
	struct page *p_page;

	p_page = buddy_get_pages(&global_mem, order);
	return page_to_virt(&global_mem, p_page);
}

void free_pages(void *addr)
{
	struct page *p_page;
	p_page = virt_to_page(&global_mem, addr);
	buddy_free_pages(&global_mem, p_page);
}
