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

#include <common/macro.h>
#include <common/util.h>
#include <common/list.h>
#include <common/errno.h>
#include <common/kprint.h>
#include <mm/vmspace.h>
#include <common/kmalloc.h>
#include <common/mm.h>
#include <common/mmu.h>

/* local functions */

static struct vmregion *alloc_vmregion(void)
{
	struct vmregion *vmr;

	vmr = kmalloc(sizeof(*vmr));
	return vmr;
}

static void free_vmregion(struct vmregion *vmr)
{
	kfree((void *)vmr);
}

/*
 * Returns 0 when no intersection detected.
 */
static int check_vmr_intersect(struct vmspace *vmspace,
			       struct vmregion *vmr_to_add)
{
	struct vmregion *vmr;
	vaddr_t new_start, start;
	vaddr_t new_end, end;

	new_start = vmr_to_add->start;
	new_end = new_start + vmr_to_add->size - 1;

	for_each_in_list(vmr, struct vmregion, node, &(vmspace->vmr_list)) {
		start = vmr->start;
		end = start + vmr->size;
		if ((new_start >= start && new_start < end) ||
		    (new_end >= start && new_end < end))
			return 1;
	}
	return 0;
}

static int is_vmr_in_vmspace(struct vmspace *vmspace, struct vmregion *vmr)
{
	struct vmregion *iter;

	for_each_in_list(iter, struct vmregion, node, &(vmspace->vmr_list)) {
		if (iter == vmr)
			return 1;
	}
	return 0;
}

static int add_vmr_to_vmspace(struct vmspace *vmspace, struct vmregion *vmr)
{
	if (check_vmr_intersect(vmspace, vmr) != 0) {
		printk("warning: vmr overlap\n");
		return -EINVAL;
	}
	list_add(&(vmr->node), &(vmspace->vmr_list));
	return 0;
}

static void del_vmr_from_vmspace(struct vmspace *vmspace, struct vmregion *vmr)
{
	if (is_vmr_in_vmspace(vmspace, vmr))
		list_del(&(vmr->node));
	free_vmregion(vmr);
}

struct vmregion *find_vmr_for_va(struct vmspace *vmspace, vaddr_t addr)
{
	struct vmregion *vmr;
	vaddr_t start, end;

	for_each_in_list(vmr, struct vmregion, node, &(vmspace->vmr_list)) {
		start = vmr->start;
		end = start + vmr->size;
		if (addr >= start && addr < end)
			return vmr;
	}
	return NULL;
}

static int fill_page_table(struct vmspace *vmspace, struct vmregion *vmr)
{
	size_t pm_size;
	paddr_t pa;
	vaddr_t va;
	int ret;

	pm_size = vmr->pmo->size;
	pa = vmr->pmo->start;
	va = vmr->start;

	ret = map_range_in_pgtbl(vmspace->pgtbl, va, pa, pm_size, vmr->perm);

	return ret;
}

int vmspace_map_range(struct vmspace *vmspace, vaddr_t va, size_t len,
		      vmr_prop_t flags, struct pmobject *pmo)
{
	struct vmregion *vmr;
	int ret;

	va = ROUND_DOWN(va, PAGE_SIZE);
	if (len < PAGE_SIZE)
		len = PAGE_SIZE;

	vmr = alloc_vmregion();
	if (!vmr) {
		ret = -ENOMEM;
		goto out_fail;
	}
	vmr->start = va;
	vmr->size = len;
	vmr->perm = flags;
	vmr->pmo = pmo;

	ret = add_vmr_to_vmspace(vmspace, vmr);

	if (ret < 0)
		goto out_free_vmr;
	BUG_ON((pmo->type != PMO_DATA) &&
	       (pmo->type != PMO_ANONYM) &&
	       (pmo->type != PMO_DEVICE) && (pmo->type != PMO_SHM));
	/* on-demand mapping for anonymous mapping */
	if (pmo->type == PMO_DATA)
		fill_page_table(vmspace, vmr);
	return 0;
 out_free_vmr:
	free_vmregion(vmr);
 out_fail:
	return ret;
}

struct vmregion *init_heap_vmr(struct vmspace *vmspace, vaddr_t va,
			       struct pmobject *pmo)
{
	struct vmregion *vmr;
	int ret;

	vmr = alloc_vmregion();
	if (!vmr) {
		kwarn("%s fails\n", __func__);
		goto out_fail;
	}
	vmr->start = va;
	vmr->size = 0;
	vmr->perm = VMR_READ | VMR_WRITE;
	vmr->pmo = pmo;

	ret = add_vmr_to_vmspace(vmspace, vmr);

	if (ret < 0)
		goto out_free_vmr;

	return vmr;

 out_free_vmr:
	free_vmregion(vmr);
 out_fail:
	return NULL;
}

int vmspace_unmap_range(struct vmspace *vmspace, vaddr_t va, size_t len)
{
	struct vmregion *vmr;
	vaddr_t start;
	size_t size;

	vmr = find_vmr_for_va(vmspace, va);
	if (!vmr)
		return -1;
	start = vmr->start;
	size = vmr->size;

	if ((va != start) && (len != size)) {
		printk("we only support unmap a whole vmregion now.\n");
		BUG_ON(1);
	}

	del_vmr_from_vmspace(vmspace, vmr);

	unmap_range_in_pgtbl(vmspace->pgtbl, va, len);

	return 0;
}

#define HEAP_START (0x600000000000)

int vmspace_init(struct vmspace *vmspace)
{
	init_list_head(&vmspace->vmr_list);
	/* alloc the root page table page */
	vmspace->pgtbl = get_pages(0);
	BUG_ON(vmspace->pgtbl == NULL);
	memset((void *)vmspace->pgtbl, 0, PAGE_SIZE);

	/* architecture dependent initilization */
	vmspace->user_current_heap = HEAP_START;

	return 0;
}

/* release the resource when a process exits */
int destroy_vmspace(struct vmspace *vmspace)
{
	// unmap each vmregion in vmspace->vmr_list
	struct vmregion *vmr;
	vaddr_t start;
	size_t size;

	for_each_in_list(vmr, struct vmregion, node, &(vmspace->vmr_list)) {
		start = vmr->start;
		size = vmr->size;
		del_vmr_from_vmspace(vmspace, vmr);
		unmap_range_in_pgtbl(vmspace->pgtbl, start, size);
	}

	kfree(vmspace);
	return 0;
}

/*
 * @paddr is only useful when @type == PMO_DEVICE.
 */
/* init an allocated pmobject */
void pmo_init(struct pmobject *pmo, pmo_type_t type, size_t len, paddr_t paddr)
{
	memset((void *)pmo, 0, sizeof(*pmo));

	len = ROUND_UP(len, PAGE_SIZE);
	pmo->size = len;
	pmo->type = type;

	/* for a PMO_DATA, the user will use it soon (we expect) */
	if (type == PMO_DATA) {
		/* kmalloc(>2048) returns continous physical pages */
		pmo->start = (paddr_t) virt_to_phys(kmalloc(len));
	} else if (type == PMO_DEVICE) {
		pmo->start = paddr;
	} else {
		/*
		 * for stack, heap, we do not allocate the physical memory at
		 * once
		 */
		pmo->radix = new_radix();
		init_radix(pmo->radix);
	}
}

void commit_page_to_pmo(struct pmobject *pmo, u64 index, paddr_t pa)
{
	int ret;

	BUG_ON(pmo->type != PMO_ANONYM);
	ret = radix_add(pmo->radix, index, (void *)pa);
	BUG_ON(ret != 0);
}

/* return 0 (NULL) when not found */
paddr_t get_page_from_pmo(struct pmobject *pmo, u64 index)
{
	paddr_t pa;

	pa = (paddr_t) radix_get(pmo->radix, index);
	return pa;
}

/* switch vmspace */
void switch_vmspace_to(struct vmspace *vmspace)
{
	set_page_table(virt_to_phys(vmspace->pgtbl));
}
