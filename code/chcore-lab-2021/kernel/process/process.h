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

#include <process/capability.h>
#include <common/list.h>
#include <common/types.h>
#include <common/bitops.h>
#include <common/kprint.h>
#include <common/macro.h>
#include <common/sync.h>

struct object_slot {
	u64 slot_id;
	struct process *process;
	int isvalid;
	u64 rights;
	struct object *object;
	/* link copied slots pointing to the same object */
	struct list_head copies;
};

#define BASE_OBJECT_NUM		BITS_PER_LONG
/* 1st cap is process. 2nd cap is vmspace */
#define PROCESS_OBJ_ID		0
#define VMSPACE_OBJ_ID		1

struct slot_table {
	unsigned int slots_size;
	struct object_slot **slots;
	/*
	 * if a bit in full_slots_bmp is 1, corresponding
	 * sizeof(unsigned long) bits in slots_bmp are all set
	 */
	unsigned long *full_slots_bmp;
	unsigned long *slots_bmp;
};

struct process {
	struct slot_table slot_table;

	struct list_head thread_list;
};

#define current_process (current_thread->process)

void process_create_root(char *bin_name);
/*
 * ATTENTION: These interfaces are for capability internal use.
 * As a cap user, check capability.h for interfaces for cap.
 */
int alloc_slot_id(struct process *process);

static inline void free_slot_id(struct process *process, int slot_id)
{
	struct slot_table *slot_table = &process->slot_table;
	clear_bit(slot_id, slot_table->slots_bmp);
	clear_bit(slot_id / BITS_PER_LONG, slot_table->full_slots_bmp);
	slot_table->slots[slot_id] = NULL;
}

static inline struct object_slot *get_slot(struct process *process, int slot_id)
{
	if (slot_id < 0 || slot_id >= process->slot_table.slots_size)
		return NULL;
	return process->slot_table.slots[slot_id];
}

static inline void install_slot(struct process *process, int slot_id,
				struct object_slot *slot)
{
	BUG_ON(!get_bit(slot_id, process->slot_table.slots_bmp));
	process->slot_table.slots[slot_id] = slot;
}

bool is_valid_slot_id(struct slot_table *slot_table, int slot_id);

void process_exit(struct process *process);
