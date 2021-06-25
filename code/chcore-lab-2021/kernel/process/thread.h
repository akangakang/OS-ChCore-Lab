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
#include <mm/vmspace.h>
#include <sched/sched.h>
#include <process/process.h>
#include <common/smp.h>
#include <ipc/ipc.h>

extern struct thread *current_threads[PLAT_CPU_NUM];
#define current_thread (current_threads[smp_get_cpu_id()])
#define DEFAULT_KERNEL_STACK_SZ		(0x1000)

/* Arguments for the inital thread */
#define ROOT_THREAD_STACK_BASE		(0x8000000)
#define ROOT_THREAD_STACK_SIZE		(0x10000)
#define ROOT_THREAD_PRIO		MAX_PRIO - 1

struct thread {
	struct list_head node;	// link threads in a same process
	struct list_head ready_queue_node;	// link threads in a ready queue
	struct list_head notification_queue_node;	// link threads in a notification waiting queue
	struct thread_ctx *thread_ctx;	// thread control block
	struct vmspace *vmspace;	// memory mapping

	struct process *process;

	struct ipc_connection *active_conn;
	struct server_ipc_config *server_ipc_config;
};

void switch_thread_vmspace_to(struct thread *);
void thread_deinit(void *thread_ptr);
int thread_create_main(struct process *process, u64 stack_base,
		       u64 stack_size, u32 prio, u32 type, s32 aff,
		       const char *bin_start, char *bin_name);
void sys_exit(int ret);
int thread_create(struct process *process, u64 stack, u64 pc,
		  u64 arg, u32 prio, u32 type, s32 aff);
