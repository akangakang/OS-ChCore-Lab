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

#include <common/list.h>
#include <common/mmu.h>

#include <common/radix.h>

struct file_cap {
};

struct vmregion {
	struct list_head node;	// vmr_list
	vaddr_t start;
	size_t size;
	vmr_prop_t perm;
	struct pmobject *pmo;
};

struct vmspace {
	/* list of vmregion */
	struct list_head vmr_list;
	/* root page table */
	vaddr_t *pgtbl;

	struct vmregion *heap_vmr;
	vaddr_t user_current_heap;
};

typedef u64 pmo_type_t;
#define PMO_ANONYM     0	/* lazy allocation */
#define PMO_DATA       1	/* immediate allocation */
#define PMO_FILE       2	/* file backed */
#define PMO_SHM        3	/* shared memory */
#define PMO_USER_PAGER 4	/* support user pager */
#define PMO_DEVICE     5	/* memory mapped device registers */

struct pmobject {
	struct radix *radix;	/* record physical pages */
	paddr_t start;
	size_t size;
	pmo_type_t type;
	atomic_cnt refcnt;

	// if type == PMO_BACKED
	struct file_cap *file;
	off_t offset;
};

int vmspace_init(struct vmspace *vmspace);
void pmo_init(struct pmobject *pmo, pmo_type_t type, size_t len, paddr_t paddr);

int vmspace_map_range(struct vmspace *vmspace, vaddr_t va, size_t len,
		      vmr_prop_t flags, struct pmobject *pmo);
int vmspace_unmap_range(struct vmspace *vmspace, vaddr_t va, size_t len);

struct vmregion *find_vmr_for_va(struct vmspace *vmspace, vaddr_t addr);

void switch_vmspace_to(struct vmspace *);

void commit_page_to_pmo(struct pmobject *pmo, u64 index, paddr_t pa);
paddr_t get_page_from_pmo(struct pmobject *pmo, u64 index);

struct vmregion *init_heap_vmr(struct vmspace *vmspace, vaddr_t va,
			       struct pmobject *pmo);
