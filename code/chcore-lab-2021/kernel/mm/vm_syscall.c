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

#include <mm/vmspace.h>
#include <common/uaccess.h>
#include <common/mm.h>
#include <common/kmalloc.h>

#include <process/capability.h>
#include <process/thread.h>

int sys_create_device_pmo(u64 paddr, u64 size)
{
	int cap, r;
	struct pmobject *pmo;

	BUG_ON(size == 0);
	pmo = obj_alloc(TYPE_PMO, sizeof(*pmo));
	if (!pmo)
	{
		r = -ENOMEM;
		goto out_fail;
	}
	pmo_init(pmo, PMO_DEVICE, size, paddr);
	cap = cap_alloc(current_process, pmo, 0);
	if (cap < 0)
	{
		r = cap;
		goto out_free_obj;
	}

	return cap;
out_free_obj:
	obj_free(pmo);
out_fail:
	return r;
}

int sys_create_pmo(u64 size, u64 type)
{
	int cap, r;
	struct pmobject *pmo;

	BUG_ON(size == 0);
	pmo = obj_alloc(TYPE_PMO, sizeof(*pmo));
	if (!pmo)
	{
		r = -ENOMEM;
		goto out_fail;
	}
	pmo_init(pmo, type, size, 0);
	cap = cap_alloc(current_process, pmo, 0);
	if (cap < 0)
	{
		r = cap;
		goto out_free_obj;
	}

	return cap;
out_free_obj:
	obj_free(pmo);
out_fail:
	return r;
}

struct pmo_request
{
	/* args */
	u64 size;
	u64 type;
	/* return value */
	u64 ret_cap;
};

#define MAX_CNT 32

int sys_create_pmos(u64 user_buf, u64 cnt)
{
	u64 size;
	struct pmo_request *requests;
	int i;
	int cap;

	/* in case of integer overflow */
	if (cnt > MAX_CNT)
	{
		kwarn("create too many pmos for one time (max: %d)\n", MAX_CNT);
		return -EINVAL;
	}

	size = sizeof(*requests) * cnt;
	requests = (struct pmo_request *)kmalloc(size);
	if (requests == NULL)
	{
		kwarn("cannot allocate more memory\n");
		return -EAGAIN;
	}
	copy_from_user((char *)requests, (char *)user_buf, size);

	for (i = 0; i < cnt; ++i)
	{
		cap = sys_create_pmo(requests[i].size, requests[i].type);
		requests[i].ret_cap = cap;
	}

	/* return pmo_caps */
	copy_to_user((char *)user_buf, (char *)requests, size);

	/* free temporary buffer */
	kfree(requests);

	return 0;
}

#define WRITE_PMO 0
#define READ_PMO 1
/*
 * 如果type == WRITE_PMO，就把user_buf拷size大小到pmo->start+offset的地方
 */
static int read_write_pmo(u64 pmo_cap, u64 offset, u64 user_buf,
						  u64 size, u64 type)
{
	struct pmobject *pmo;
	int r = 0;

	/* caller should have the pmo_cap */
	pmo = obj_get(current_process, pmo_cap, TYPE_PMO);
	if (!pmo)
	{
		r = -ECAPBILITY;
		goto out_fail;
	}

	/* we only allow writing PMO_DATA now. */
	if (pmo->type != PMO_DATA)
	{
		r = -EINVAL;
		kdebug("pmo->type != PMO_DATA\n");
		goto out_obj_put;
	}

	if (offset + size < offset || offset + size > pmo->size)
	{
		r = -EINVAL;
		kdebug("offset + size < offset || offset + size > pmo->size. offset:%lx , size:%lx , pmo->size:%lx\n",offset,size,pmo->size);
		goto out_obj_put;
	}

	if (type == WRITE_PMO)
		r = copy_from_user((char *)phys_to_virt(pmo->start) + offset,
						   (char *)user_buf, size);
	else if (type == READ_PMO)
		r = copy_to_user((char *)user_buf,
						 (char *)phys_to_virt(pmo->start) + offset,
						 size);
	else
		BUG("read write pmo invalid type\n");

out_obj_put:
	obj_put(pmo);
out_fail:
	return r;
}

/*
 * A process can send a PMO (with msgs) to others.
 * It can write the msgs without mapping the PMO with this function.
 */
int sys_write_pmo(u64 pmo_cap, u64 offset, u64 user_ptr, u64 len)
{
	return read_write_pmo(pmo_cap, offset, user_ptr, len, WRITE_PMO);
}

int sys_read_pmo(u64 pmo_cap, u64 offset, u64 user_ptr, u64 len)
{
	return read_write_pmo(pmo_cap, offset, user_ptr, len, READ_PMO);
}

/*
 * A process can not only map a PMO into its private address space,
 * but also can map a PMO to some others (e.g., load code for others).
 */
int sys_map_pmo(u64 target_process_cap, u64 pmo_cap, u64 addr, u64 perm)
{
	struct vmspace *vmspace;
	struct pmobject *pmo;
	struct process *target_process;
	int r;

	pmo = obj_get(current_process, pmo_cap, TYPE_PMO);
	if (!pmo)
	{
		r = -ECAPBILITY;
		goto out_fail;
	}

	/* map the pmo to the target process */
	target_process = obj_get(current_process, target_process_cap,
							 TYPE_PROCESS);
	if (!target_process)
	{
		r = -ECAPBILITY;
		goto out_obj_put_pmo;
	}
	vmspace = obj_get(target_process, VMSPACE_OBJ_ID, TYPE_VMSPACE);
	BUG_ON(vmspace == NULL);

	r = vmspace_map_range(vmspace, addr, pmo->size, perm, pmo);
	if (r != 0)
	{
		r = -EPERM;
		goto out_obj_put_vmspace;
	}

	/*
	 * when a process maps a pmo to others,
	 * this func returns the new_cap in the target process.
	 */
	if (target_process != current_process)
		/* if using cap_move, we need to consider remove the mappings */
		r = cap_copy(current_process, target_process, pmo_cap, 0, 0);
	else
		r = 0;

out_obj_put_vmspace:
	obj_put(vmspace);
	obj_put(target_process);
out_obj_put_pmo:
	obj_put(pmo);
out_fail:
	return r;
}

struct pmo_map_request
{
	/* args */
	u64 pmo_cap;
	u64 addr;
	u64 perm;

	/* return caps or return value */
	u64 ret;
};

int sys_map_pmos(u64 target_process_cap, u64 user_buf, u64 cnt)
{
	u64 size;
	struct pmo_map_request *requests;
	int i;
	int ret;

	/* in case of integer overflow */
	if (cnt > MAX_CNT)
	{
		kwarn("create too many pmos for one time (max: %d)\n", MAX_CNT);
		return -EINVAL;
	}

	size = sizeof(*requests) * cnt;
	requests = (struct pmo_map_request *)kmalloc(size);
	if (requests == NULL)
	{
		kwarn("cannot allocate more memory\n");
		return -EAGAIN;
	}
	copy_from_user((char *)requests, (char *)user_buf, size);

	for (i = 0; i < cnt; ++i)
	{
		/*
		 * if target_process is not current_process,
		 * ret is cap on success.
		 */
		ret = sys_map_pmo(target_process_cap,
						  requests[i].pmo_cap,
						  requests[i].addr, requests[i].perm);
		requests[i].ret = ret;
	}

	copy_to_user((char *)user_buf, (char *)requests, size);

	kfree(requests);
	return 0;
}

int sys_unmap_pmo(u64 target_process_cap, u64 pmo_cap, u64 addr)
{
	struct vmspace *vmspace;
	struct pmobject *pmo;
	struct process *target_process;
	int ret;

	/* caller should have the pmo_cap */
	pmo = obj_get(current_process, pmo_cap, TYPE_PMO);
	if (!pmo)
		return -EPERM;

	/* map the pmo to the target process */
	target_process = obj_get(current_process, target_process_cap,
							 TYPE_PROCESS);
	if (!target_process)
	{
		ret = -EPERM;
		goto fail1;
	}

	vmspace = obj_get(target_process, VMSPACE_OBJ_ID, TYPE_VMSPACE);
	if (!vmspace)
	{
		ret = -EPERM;
		goto fail2;
	}

	ret = vmspace_unmap_range(vmspace, addr, pmo->size);
	if (ret != 0)
		ret = -EPERM;

	obj_put(vmspace);
fail2:
	obj_put(target_process);
fail1:
	obj_put(pmo);

	return ret;
}

/*
 * User process heap start: 0x600000000000
 *
 * defined in mm/vmregion.c
 */

u64 sys_handle_brk(u64 addr)
{
	struct vmspace *vmspace;
	struct pmobject *pmo;
	struct vmregion *vmr;
	// size_t len;
	u64 retval;
	// int ret;

	vmspace = obj_get(current_process, VMSPACE_OBJ_ID, TYPE_VMSPACE);

	/*
	 * Lab3: Your code here
	 * The sys_handle_brk syscall modifies the top address of heap to addr.
	 *
	 * If addr is 0, this function should initialize the heap, implemeted by:
	 * 1. Create a new pmo with size 0 and type PMO_ANONYM.
	 * 2. Initialize vmspace->heap_vmr using function init_heap_vmr(), which generates 
	 * the mapping between  user heap's virtual address (already stored in 
	 * vmspace->user_current_heap) and the pmo you just created.
	 *
	 * HINT: For more details about how to create and initiailze a pmo, check function 
	 * 'load_binary' for reference.
	 *
	 * If addr is larger than heap, the size of vmspace->heap_vmr and the size of its 
	 * coresponding pmo should be updated. Real physical memory allocation are done in 
	 * a lazy manner using pagefault handler later at the first access time. 
	 *
	 * If addr is smaller than heap, do nothing and return -EINVAL since we are not going
	 * to support shink heap. For all other cases, return the virtual address of the heap 
	 * top.
	 *
	 */

	/*
	 * return origin heap addr on failure;
	 * return new heap addr on success.
	 */

	/*  initialize the heap */
	if (addr == 0)
	{
		/* 1. Create a new pmo with size 0 and type PMO_ANONYM. */
		pmo = obj_alloc(TYPE_PMO, sizeof(*pmo));
		if (!pmo)
		{
			retval = -ENOMEM;
			goto error;
		}
		pmo_init(pmo, PMO_ANONYM, 0, 0);
		int pmo_cap = cap_alloc(current_process, pmo, 0);
		if (pmo_cap < 0)
		{
			retval = pmo_cap;
			goto error;
		}

		/* 2. Initialize vmspace->heap_vmr using function init_heap_vmr() */
		vmr = init_heap_vmr(vmspace, vmspace->user_current_heap, pmo);
		if (vmr == NULL)
		{
			/* same reason as init_heap_vmr return null */
			retval = -EINVAL;
			goto error;
		}

		vmspace->heap_vmr = vmr;
		retval = vmspace->user_current_heap;
		return retval;
	}

	if (addr >= (vmspace->user_current_heap + vmspace->heap_vmr->size))
	{
		vmr = vmspace->heap_vmr;
		int new_size = ROUND_UP((addr-vmspace->user_current_heap),PAGE_SIZE);
		vmr->pmo->size= new_size;
		vmspace ->heap_vmr->size=new_size;
		retval = addr;

	}
	else
	{
		retval = -EINVAL;
		goto error;
	}

error:
	obj_put(vmspace);
	return retval;
}
