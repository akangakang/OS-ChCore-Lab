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

#include <process/capability.h>
#include <process/process.h>
#include <process/thread.h>
#include <common/kmalloc.h>
#include <common/uaccess.h>
#include <common/printk.h>

const obj_deinit_func obj_deinit_tbl[TYPE_NR] = {
	[0 ... TYPE_NR - 1] = NULL,
	[TYPE_THREAD] = thread_deinit,
};

/* local object operation methods */
static void *get_opaque(struct process *process, int slot_id,
			bool type_valid, int type)
{
	struct slot_table *slot_table = &process->slot_table;
	struct object_slot *slot;
	void *obj;

	if (!is_valid_slot_id(slot_table, slot_id)) {
		obj = NULL;
		goto out_unlock_table;
	}

	slot = get_slot(process, slot_id);
	BUG_ON(slot->isvalid == false);
	BUG_ON(slot->object == NULL);

	if (!type_valid || slot->object->type == type) {
		obj = slot->object->opaque;
	} else {
		obj = NULL;
		goto out_unlock_slot;
	}

	atomic_fetch_add_64(&slot->object->refcount, 1);

 out_unlock_slot:
 out_unlock_table:
	return obj;
}

static struct object *__object_get(struct process *process, int slot_id)
{
	void *obj;
	obj = get_opaque(process, slot_id, false, 0);
	if (!obj)
		return NULL;
	else
		return container_of(obj, struct object, opaque);
}

static void __object_put(struct object *object)
{
	u64 old_refcount;
	old_refcount = atomic_fetch_sub_64(&object->refcount, 1);
	if (old_refcount == 1)
		kfree(object);
}

/* object refenrence */
void *obj_get(struct process *process, int slot_id, int type)
{
	return get_opaque(process, slot_id, true, type);
}

void obj_put(void *obj)
{
	struct object *object = container_of(obj, struct object, opaque);
	__object_put(object);
}

void *obj_alloc(u64 type, u64 size)
{
	u64 total_size;
	struct object *object;

	// opaque is u64 so sizeof(*object) is 8-byte aligned.
	//      Thus the address of object-defined data is always 8-byte aligned.
	total_size = sizeof(*object) + size;
	object = kmalloc(total_size);
	if (!object)
		return NULL;

	object->type = type;
	object->size = size;
	object->refcount = 0;
	init_list_head(&object->copies_head);

	return object->opaque;
}

/*
 * obj_free can be used in two conditions:
 * 1. In initialization of a cap (after obj_alloc and before cap_aloc) as a
 * fallback to obj_alloc
 * 2. To all slots which point to it. In this use case, the
 * caller must guarantee that the object is not freed (i.e., should hold a
 * reference to it).
 */
void obj_free(void *obj)
{
	struct object *object;
	struct object_slot *slot_iter = NULL, *slot_iter_tmp = NULL;
	int r;

	if (!obj)
		return;
	object = container_of(obj, struct object, opaque);

	/* fallback of obj_alloc */
	if (object->refcount == 0) {
		kfree(object);
		return;
	}

	/* free all copied slots */
	for_each_in_list_safe(slot_iter, slot_iter_tmp,
			      copies, &object->copies_head) {
		u64 iter_slot_id = slot_iter->slot_id;
		struct process *iter_process = slot_iter->process;

		r = cap_free(iter_process, iter_slot_id);
		BUG_ON(r != 0);
	}
}

int cap_alloc(struct process *process, void *obj, u64 rights)
{
	struct object *object;
	struct object_slot *slot;
	int r, slot_id;

	object = container_of(obj, struct object, opaque);

	slot_id = alloc_slot_id(process);
	if (slot_id < 0) {
		r = -ENOMEM;
		goto out_unlock_table;
	}

	slot = kmalloc(sizeof(*slot));
	if (!slot) {
		r = -ENOMEM;
		goto out_free_slot_id;
	}
	slot->slot_id = slot_id;
	slot->process = process;
	slot->isvalid = true;
	slot->rights = rights;
	slot->object = object;
	list_add(&slot->copies, &object->copies_head);

	BUG_ON(object->refcount != 0);
	object->refcount = 1;

	install_slot(process, slot_id, slot);

	return slot_id;
 out_free_slot_id:
	free_slot_id(process, slot_id);
 out_unlock_table:
	return r;
}

int cap_free(struct process *process, int slot_id)
{
	struct object_slot *slot;
	struct object *object;
	int r = 0;
	u64 old_refcount;
	obj_deinit_func func;

	slot = get_slot(process, slot_id);
	if (!slot || slot->isvalid == false) {
		r = -ECAPBILITY;
		goto out_unlock_table;
	}

	free_slot_id(process, slot_id);
	/* no need to get slot_guard as it can not be accessed */

	object = slot->object;
	old_refcount = atomic_fetch_sub_64(&object->refcount, 1);
	if (old_refcount == 1) {
		func = obj_deinit_tbl[object->type];
		if (func)
			func(object->opaque);
		if (object->refcount == 0)
			kfree(object);
	}

	slot->isvalid = false;
	slot->object = NULL;
	list_del(&slot->copies);
	kfree(slot);

	return r;
 out_unlock_table:
	return r;
}

int cap_copy(struct process *src_process, struct process *dest_process,
	     int src_slot_id, bool new_rights_valid, u64 new_rights)
{
	struct object_slot *src_slot, *dest_slot;
	int r, dest_slot_id;

	src_slot = get_slot(src_process, src_slot_id);
	if (!src_slot || src_slot->isvalid == false) {
		r = -ECAPBILITY;
		goto out_unlock;
	}

	dest_slot_id = alloc_slot_id(dest_process);
	if (dest_slot_id == -1) {
		r = -ENOMEM;
		goto out_unlock;
	}

	dest_slot = kmalloc(sizeof(*dest_slot));
	if (!dest_slot) {
		r = -ENOMEM;
		goto out_free_slot_id;
	}
	src_slot = get_slot(src_process, src_slot_id);
	atomic_fetch_add_64(&src_slot->object->refcount, 1);

	dest_slot->slot_id = dest_slot_id;
	dest_slot->process = dest_process;
	dest_slot->isvalid = true;
	dest_slot->object = src_slot->object;
	dest_slot->rights = new_rights_valid ? new_rights : src_slot->rights;
	list_add(&dest_slot->copies, &src_slot->copies);

	install_slot(dest_process, dest_slot_id, dest_slot);

	return dest_slot_id;
 out_free_slot_id:
	free_slot_id(dest_process, dest_slot_id);
 out_unlock:
	return r;
}

/*
 * Copy capability within the same process.
 * Returns new cap or error code.
 */
int cap_copy_local(struct process *process, int src_slot_id, u64 new_rights)
{
	return cap_copy(process, process, src_slot_id, true, new_rights);
}

int cap_move(struct process *src_process, struct process *dest_process,
	     int src_slot_id, bool new_rights_valid, u64 new_rights)
{
	int r;

	r = cap_copy(src_process, dest_process, src_slot_id,
		     new_rights_valid, new_rights);
	if (r < 0)
		return r;
	r = cap_free(src_process, src_slot_id);
	BUG_ON(r);		/* if copied successfully, free should not fail */

	return r;
}

int cap_revoke(struct process *process, int slot_id)
{
	struct object *object;
	int r = 0;

	object = __object_get(process, slot_id);
	if (!object) {
		return -ECAPBILITY;
	}
	obj_free(object->opaque);

	__object_put(object);
	return r;
}

int sys_cap_copy_to(u64 dest_process_cap, u64 src_slot_id)
{
	struct process *dest_process;
	int r;

	dest_process = obj_get(current_process, dest_process_cap, TYPE_PROCESS);
	if (!dest_process)
		return -ECAPBILITY;
	r = cap_copy(current_process, dest_process, src_slot_id, 0, 0);
	obj_put(dest_process);
	return r;
}

int sys_cap_copy_from(u64 src_process_cap, u64 src_slot_id)
{
	struct process *src_process;
	int r;

	src_process = obj_get(current_process, src_process_cap, TYPE_PROCESS);
	if (!src_process)
		return -ECAPBILITY;
	r = cap_copy(src_process, current_process, src_slot_id, 0, 0);
	obj_put(src_process);
	return r;
}

int sys_transfer_caps(u64 dest_group_cap, u64 src_caps_buf, int nr_caps,
		      u64 dst_caps_buf)
{
	struct process *dest_process;
	int i;
	int *src_caps;
	int *dst_caps;
	size_t size;

	dest_process = obj_get(current_process, dest_group_cap, TYPE_PROCESS);
	if (!dest_process)
		return -ECAPBILITY;

	size = sizeof(int) * nr_caps;
	src_caps = kmalloc(size);
	dst_caps = kmalloc(size);

	/* get args from user buffer */
	copy_from_user((void *)src_caps, (void *)src_caps_buf, size);

	for (i = 0; i < nr_caps; ++i) {
		dst_caps[i] = cap_copy(current_process, dest_process,
				       src_caps[i], 0, 0);
	}

	/* write results to user buffer */
	copy_to_user((void *)dst_caps_buf, (void *)dst_caps, size);

	obj_put(dest_process);
	return 0;
}

int sys_cap_move(u64 dest_process_cap, u64 src_slot_id)
{
	struct process *dest_process;
	int r;

	dest_process = obj_get(current_process, dest_process_cap, TYPE_PROCESS);
	if (!dest_process)
		return -ECAPBILITY;
	r = cap_move(current_process, dest_process, src_slot_id, 0, 0);
	obj_put(dest_process);
	return r;
}

// for debug
int sys_get_all_caps(u64 process_cap)
{
	struct process *process;
	struct slot_table *slot_table;
	int i;

	process = obj_get(current_process, process_cap, TYPE_PROCESS);
	if (!process)
		return -ECAPBILITY;
	printk("thread %p cap:\n", current_thread);

	slot_table = &process->slot_table;
	for (i = 0; i < slot_table->slots_size; i++) {
		struct object_slot *slot = get_slot(process, i);
		if (!slot)
			continue;
		BUG_ON(slot->isvalid != true);
		printk("slot_id:%d type:%d\n", i,
		       slot_table->slots[i]->object->type);
	}

	obj_put(process);
	return 0;
}
