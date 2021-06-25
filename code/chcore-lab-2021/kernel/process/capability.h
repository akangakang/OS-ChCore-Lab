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

#include <common/types.h>
#include <common/errno.h>
#include <common/list.h>

struct object {
	u64 type;
	u64 size;
	/* Link all slots point to this object */
	struct list_head copies_head;
	/*
	 * refcount is added when a slot points to it and when get_object is
	 * called. Object is freed when it reaches 0.
	 */
	u64 refcount;
	u64 opaque[];
};

enum object_type {
	TYPE_PROCESS = 0,
	TYPE_THREAD,
	TYPE_CONNECTION,
	TYPE_NOTIFICATION,
	TYPE_PMO,
	TYPE_VMSPACE,
	TYPE_NR,
};

struct process;

typedef void (*obj_deinit_func) (void *);
extern const obj_deinit_func obj_deinit_tbl[TYPE_NR];

void *obj_get(struct process *process, int slot_id, int type);
void obj_put(void *obj);
void *obj_alloc(u64 type, u64 size);
void obj_free(void *obj);

int cap_alloc(struct process *process, void *obj, u64 rights);
int cap_free(struct process *process, int slot_id);
int cap_copy(struct process *src_process, struct process *dest_process,
	     int src_slot_id, bool new_rights_valid, u64 new_rights);
int cap_copy_local(struct process *process, int src_slot_id, u64 new_rights);
int cap_move(struct process *src_process, struct process *dest_process,
	     int src_slot_id, bool new_rights_valid, u64 new_rights);
int cap_revoke(struct process *process, int slot_id);
