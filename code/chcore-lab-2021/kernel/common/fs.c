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
#include <common/fs.h>
#include <process/capability.h>
#include <process/process.h>
#include <process/thread.h>
#include <common/uaccess.h>
#include <common/kmalloc.h>
#include <common/mm.h>
#include <mm/vmspace.h>

int sys_fs_load_cpio(u64 vaddr)
{
	struct pmobject *cpio_pmo;
	struct vmspace *vmspace;
	int cpio_pmo_cap, ret;
	size_t len;

	cpio_pmo = obj_alloc(TYPE_PMO, sizeof(*cpio_pmo));
	if (!cpio_pmo) {
		return -ENOMEM;
	}
	len = ROUND_UP(binary_cpio_bin_size, PAGE_SIZE);
	pmo_init(cpio_pmo, PMO_DATA, len, 0);

	cpio_pmo_cap = cap_alloc(current_process, cpio_pmo, 0);
	if (cpio_pmo_cap < 0) {
		obj_free(cpio_pmo);
		return -ENOMEM;
	}

	vmspace = obj_get(current_process, VMSPACE_OBJ_ID, TYPE_VMSPACE);

	ret = vmspace_map_range(vmspace, vaddr, len, VMR_READ, cpio_pmo);
	memcpy((void *)phys_to_virt(cpio_pmo->start),
	       &binary_cpio_bin_start, binary_cpio_bin_size);

	obj_put(vmspace);
	return ret;
}
