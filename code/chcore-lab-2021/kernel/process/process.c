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

#include <process/process.h>
#include <process/thread.h>
#include <common/list.h>
#include <common/util.h>
#include <common/bitops.h>
#include <common/kmalloc.h>
#include <mm/vmspace.h>
#include <common/printk.h>
#include <common/cpio.h>

/* tool functions */
bool is_valid_slot_id(struct slot_table * slot_table, int slot_id)
{
	if (slot_id < 0 && slot_id >= slot_table->slots_size)
		return false;
	if (!get_bit(slot_id, slot_table->slots_bmp))
		return false;
	if (slot_table->slots[slot_id] == NULL)
		BUG("slot NULL while bmp is not\n");
	return true;
}

static int slot_table_init(struct slot_table *slot_table, unsigned int size)
{
	int r;

	size = DIV_ROUND_UP(size, BASE_OBJECT_NUM) * BASE_OBJECT_NUM;
	slot_table->slots_size = size;

	slot_table->slots = kzalloc(size * sizeof(*slot_table->slots));
	if (!slot_table->slots) {
		r = -ENOMEM;
		goto out_fail;
	}

	slot_table->slots_bmp = kzalloc(BITS_TO_LONGS(size)
					* sizeof(unsigned long));
	if (!slot_table->slots_bmp) {
		r = -ENOMEM;
		goto out_free_slots;
	}

	slot_table->full_slots_bmp = kzalloc(BITS_TO_LONGS(BITS_TO_LONGS(size))
					     * sizeof(unsigned long));
	if (!slot_table->full_slots_bmp) {
		r = -ENOMEM;
		goto out_free_slots_bmp;
	}

	return 0;
 out_free_slots_bmp:
	kfree(slot_table->slots_bmp);
 out_free_slots:
	kfree(slot_table->slots);
 out_fail:
	return r;
}

static int expand_slot_table(struct slot_table *slot_table)
{
	unsigned int new_size, old_size;
	struct slot_table new_slot_table;
	int r;

	old_size = slot_table->slots_size;
	new_size = old_size + BASE_OBJECT_NUM;
	r = slot_table_init(&new_slot_table, new_size);
	if (r < 0)
		return r;

	memcpy(new_slot_table.slots, slot_table->slots,
	       old_size * sizeof(*slot_table->slots));
	memcpy(new_slot_table.slots_bmp, slot_table->slots_bmp,
	       BITS_TO_LONGS(old_size) * sizeof(unsigned long));
	memcpy(new_slot_table.full_slots_bmp, slot_table->full_slots_bmp,
	       BITS_TO_LONGS(BITS_TO_LONGS(old_size)) * sizeof(unsigned long));
	slot_table->slots_size = new_size;
	slot_table->slots = new_slot_table.slots;
	slot_table->slots_bmp = new_slot_table.slots_bmp;
	slot_table->full_slots_bmp = new_slot_table.full_slots_bmp;

	return 0;
}

int alloc_slot_id(struct process *process)
{
	int empty_idx = 0, r;
	struct slot_table *slot_table;
	int bmp_size = 0, full_bmp_size = 0;

	slot_table = &process->slot_table;

	while (true) {
		bmp_size = slot_table->slots_size;
		full_bmp_size = BITS_TO_LONGS(bmp_size);

		empty_idx = find_next_zero_bit(slot_table->full_slots_bmp,
					       full_bmp_size, 0);
		if (empty_idx >= full_bmp_size)
			goto expand;

		empty_idx = find_next_zero_bit(slot_table->slots_bmp,
					       bmp_size,
					       empty_idx * BITS_PER_LONG);
		if (empty_idx >= bmp_size)
			goto expand;
		else
			break;
 expand:
		r = expand_slot_table(slot_table);
		if (r < 0)
			goto out_fail;
	}
	BUG_ON(empty_idx < 0 || empty_idx >= bmp_size);

	set_bit(empty_idx, slot_table->slots_bmp);
	if (slot_table->full_slots_bmp[empty_idx / BITS_PER_LONG]
	    == ~((unsigned long)0))
		set_bit(empty_idx / BITS_PER_LONG, slot_table->full_slots_bmp);

	return empty_idx;
 out_fail:
	return r;
}

static int process_init(struct process *process, unsigned int size)
{
	struct slot_table *slot_table = &process->slot_table;

	BUG_ON(slot_table_init(slot_table, size));
	init_list_head(&process->thread_list);

	return 0;
}

static struct process *process_create(void)
{
	struct process *process;
	struct object *object;
	struct object_slot *slot;
	struct vmspace *vmspace;
	int total_size, slot_id;

	// init thread
	total_size = sizeof(*object) + sizeof(*process);
	if ((object = kmalloc(total_size)) == NULL)
		goto out_fail;
	object->type = TYPE_PROCESS;
	object->size = sizeof(*process);
	object->refcount = 1;
	process = (struct process *)object->opaque;
	process_init(process, BASE_OBJECT_NUM);

	// put the cap of the process its self on the first slot
	slot_id = alloc_slot_id(process);
	BUG_ON(slot_id != PROCESS_OBJ_ID);
	slot = kzalloc(sizeof(*slot));
	if (!slot)
		goto out_free_process;
	slot->slot_id = slot_id;
	slot->process = process;
	slot->isvalid = true;
	slot->object = object;
	init_list_head(&slot->copies);
	process->slot_table.slots[slot_id] = slot;

	vmspace = obj_alloc(TYPE_VMSPACE, sizeof(*vmspace));
	BUG_ON(!vmspace);
	vmspace_init(vmspace);
	slot_id = cap_alloc(process, vmspace, 0);
	BUG_ON(slot_id != VMSPACE_OBJ_ID);

	return process;
 out_free_process:
	kfree(process);
 out_fail:
	return NULL;
}

void process_exit(struct process *process)
{
	struct process *process_get;
	struct slot_table *slot_table;
	int slot_id;

	/* hold a reference and release all cap related to it */
	process_get = obj_get(process, PROCESS_OBJ_ID, TYPE_PROCESS);
	/* process is already freed by others */
	if (!process_get)
		return;
	/* cap_revoke(process, process_OBJ_ID); */

	slot_table = &process->slot_table;

	for_each_set_bit(slot_id, slot_table->slots_bmp, slot_table->slots_size) {
		cap_free(process, slot_id);
	}

	obj_put(process);
}

/*
 * Tool Functions to read a binary program file from ramdisk
 */
extern const char binary_cpio_bin_start;

static void *cpio_cb_file(const void *start, size_t size, void *data)
{
	char *buff = kmalloc(size);
	if (buff <= 0)
		return ERR_PTR(-ENOMEM);
	memcpy(buff, start, size);
	return buff;
}

static int ramdisk_read_file(char *path, char **buf)
{
	BUG_ON(path == NULL);

	int ret = 0;
	*buf = cpio_extract_single(&binary_cpio_bin_start, path, cpio_cb_file,
				   NULL);
	if (ret == 0)
		return 0;
	else
		return -ENOSYS;
}

/* process_create_root: create the root process */
void process_create_root(char *bin_name)
{
	struct process *root_process;
	int thread_cap;
	struct thread *root_thread;
	char *binary = NULL;
	int ret;

	ret = ramdisk_read_file(bin_name, &binary);
	BUG_ON(ret < 0);
	BUG_ON(binary == NULL);

	root_process = process_create();

	thread_cap = thread_create_main(root_process, ROOT_THREAD_STACK_BASE,
					ROOT_THREAD_STACK_SIZE,
					ROOT_THREAD_PRIO, TYPE_ROOT,
					smp_get_cpu_id(), binary, bin_name);

	root_thread = obj_get(root_process, thread_cap, TYPE_THREAD);
	/* Enqueue: put init thread into the ready queue */
	BUG_ON(sched_enqueue(root_thread));
	obj_put(root_thread);
}

/* syscalls */
int sys_create_process(void)
{
	struct process *new_process;
	struct vmspace *vmspace;
	int cap, r;

	/* cap current process */
	new_process = obj_alloc(TYPE_PROCESS, sizeof(*new_process));
	if (!new_process) {
		r = -ENOMEM;
		goto out_fail;
	}
	process_init(new_process, BASE_OBJECT_NUM);
	cap = cap_alloc(current_process, new_process, 0);
	if (cap < 0) {
		r = cap;
		goto out_free_obj_new_grp;
	}

	/* 1st cap is process */
	if (cap_copy(current_thread->process, new_process, cap, 0, 0)
	    != PROCESS_OBJ_ID) {
		printk("init process cap[0] is not process\n");
		r = -1;
		goto out_free_cap_grp_current;
	}

	/* 2st cap is vmspace */
	vmspace = obj_alloc(TYPE_VMSPACE, sizeof(*vmspace));
	if (!vmspace) {
		r = -ENOMEM;
		goto out_free_obj_vmspace;
	}
	vmspace_init(vmspace);
	r = cap_alloc(new_process, vmspace, 0);
	if (r < 0)
		goto out_free_obj_vmspace;
	else if (r != VMSPACE_OBJ_ID)
		BUG("init process cap[1] is not vmspace\n");

	return cap;
 out_free_obj_vmspace:
	obj_free(vmspace);
 out_free_cap_grp_current:
	cap_free(current_process, cap);
	new_process = NULL;
 out_free_obj_new_grp:
	obj_free(new_process);
 out_fail:
	return r;
}
