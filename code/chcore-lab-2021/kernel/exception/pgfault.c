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
#include <process/thread.h>
#include <mm/vmspace.h>
#include <common/types.h>
#include <common/errno.h>

#include <common/kprint.h>
#include <common/util.h>
#include <common/macro.h>
#include <common/mm.h>
#include <common/kmalloc.h>

#include "esr.h"

static inline vaddr_t get_fault_addr()
{
	vaddr_t addr;
	asm volatile ("mrs %0, far_el1\n\t":"=r" (addr));
	return addr;
}

int handle_trans_fault(struct vmspace *vmspace, vaddr_t fault_addr);

void do_page_fault(u64 esr, u64 fault_ins_addr)
{
	vaddr_t fault_addr;
	int fsc;		// fault status code

	fault_addr = get_fault_addr();
	fsc = GET_ESR_EL1_FSC(esr);
	switch (fsc) {
	case DFSC_TRANS_FAULT_L0:
	case DFSC_TRANS_FAULT_L1:
	case DFSC_TRANS_FAULT_L2:
	case DFSC_TRANS_FAULT_L3:{
			int ret;

			ret =
			    handle_trans_fault(current_thread->vmspace,
					       fault_addr);
			if (ret != 0) {
				kinfo("pgfault at 0x%p failed\n", fault_addr);
				sys_exit(ret);
			}
			break;
		}
	default:
		kinfo("do_page_fault: fsc is unsupported (0x%b) now\n", fsc);
		BUG_ON(1);
		break;
	}
}

int handle_trans_fault(struct vmspace *vmspace, vaddr_t fault_addr)
{
	struct vmregion *vmr;
	struct pmobject *pmo;
	paddr_t pa;
	// u64 offset;

	/*
	 * Lab3: your code here
	 * In this function, you should:
	 * 1. Get the vmregion of the fault_addr using find_vmr_for_va
	 * 2. If the pmo is not of type PMO_ANONYM, return -ENOMAPPING
	 * 3. Allocate one physical memory page for the PMO
	 * 4. Map the allocated address back to the page table
	 *
	 * NOTE: when any problem happened in this function, return
	 * -ENOMAPPING
	 * 
	 * NOTE: the real physical address of the PMO may not be
	 * continuous. In real chcore, all the physical pages of a PMO
	 * are recorded in a radix tree for easy management. Such code
	 * has been omitted in our lab for simplification.
	 */

	/* 1. Get the vmregion of the fault_addr using find_vmr_for_va */
	vmr = find_vmr_for_va(vmspace, fault_addr);
	if (vmr == NULL)
	{
		kinfo("[ERROR]: Get the vmregion of the fault_addr failed\n");
		return -ENOMAPPING;
	}

	/* 2. If the pmo is not of type PMO_ANONYM, return -ENOMAPPING */
	pmo = vmr->pmo;
	if (pmo->type != PMO_ANONYM)
	{
		kinfo("[ERROR]:  the pmo is not of type PMO_ANONYM\n");
		return -ENOMAPPING;
	}

	/* 3. Allocate one physical memory page for the PMO */
	// kdebug("Allocate one physical memory page for the PMO\n");
	// pa = (paddr_t) virt_to_phys(kmalloc(pmo->size));
	// pmo -> start = pa;

	// /* 4. Map the allocated address back to the page table */
	// kdebug("Map the allocated address back to the page table \n");
	// // if(map_range_in_pgtbl(vmspace->pgtbl, vmr->start, pa, pmo->size, vmr->perm) < 0){
	// // 	kinfo("[ERROR]:  Map the allocated address back to the page table failed\n");
	// // 	return -ENOMAPPING;
	// // }
	// if(map_range_in_pgtbl(vmspace->pgtbl, fault_addr, pa,PAGE_SIZE, vmr->perm) < 0){
	// 	return -ENOMAPPING;
	// }

	void *page = get_pages(0);
	if (page == NULL)
	{
		kinfo("handle_trans_fault get_pages failed\n");
		return -ENOMAPPING;
	}
	
	pa = (paddr_t)virt_to_phys((vaddr_t)page);

	fault_addr = ROUND_DOWN(fault_addr,PAGE_SIZE);
	int ret  = map_range_in_pgtbl(vmspace->pgtbl,fault_addr,pa,PAGE_SIZE,vmr->perm);
	if(ret<0)
	{
		kinfo("map_range_in_pgtbl failed\n");
		return -ENOMAPPING;
	}
	
	kinfo("finish handle_trans_fault\n");
	return 0;
}
