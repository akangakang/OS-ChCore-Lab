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

/* This file defines ds and interfaces related with synchronous IPC (Thread
 * migatation) */
#pragma once
#include <common/list.h>
#include <process/thread.h>

/*
 * Used in both server and client register.
 * Stack setting is invalid in client register.
 */
struct ipc_vm_config {
	u64 stack_base_addr;
	u64 stack_size;
	u64 buf_base_addr;
	u64 buf_size;
};

#define IPC_MAX_CONN_PER_SERVER 32
struct server_ipc_config {
	// I dont know how to specify the maximum callback number
	u64 callback;
	u64 max_client;
	/* bitmap for shared buffer and stack allocation */
	unsigned long *conn_bmp;
	struct ipc_vm_config vm_config;
};

struct shared_buf {
	u64 client_user_addr;	/* 这块共享内存在client的地址 */
	u64 server_user_addr;	/* 这块共享内存在server的地址 */
	u64 size;
};

struct ipc_connection {
	/* Source Thread */
	struct thread *source;
	/* Target Thread */
	struct thread *target;
	/* Conn cap in server */
	u64 server_conn_cap;
	/* Target function */
	u64 callback;

	/* Shadow server stack top */
	u64 server_stack_top;
	u64 server_stack_size;

	/* Shared buffer */
	struct shared_buf buf;
};

typedef struct ipc_msg {
	u64 server_conn_cap;
	u64 data_len;
	u64 cap_slot_number;
	u64 data_offset;
	u64 cap_slots_offset;
} ipc_msg_t;

/* syscall related to IPC */
u64 sys_register_server(u64 callback, u64 max_client, u64 vm_config_ptr);
u32 sys_register_client(u32 server_cap, u64 vm_config_ptr);
u64 sys_ipc_call(u32 conn_cap, ipc_msg_t * ipc_msg);
u64 sys_ipc_reg_call(u32 conn_cap, u64 arg);
void sys_ipc_return(u64 ret);

#define LAB4_IPC_BLANK 0
