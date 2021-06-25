/*
 * Copyright (c) 2020 Institute of Parallel And Distributed Systems (IPADS),
 * Shanghai Jiao Tong University (SJTU) OS-Lab-2020 (i.e., ChCore) is licensed
 * under the Mulan PSL v1. You can use this software according to the terms and
 * conditions of the Mulan PSL v1. You may obtain a copy of Mulan PSL v1 at:
 *   http://license.coscl.org.cn/MulanPSL
 *   THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PSL v1 for more details.
 */

#include <common/errno.h>
#include <common/kprint.h>
#include <common/macro.h>
#include <common/util.h>
#include <ipc/ipc.h>
#include <exception/irq.h>
#include <common/kmalloc.h>
#include <common/mm.h>
#include <common/uaccess.h>
#include <process/thread.h>
#include <sched/context.h>
#include <sched/sched.h>

#define SHADOW_THREAD_PRIO MAX_PRIO - 1

/**
 * The core function for registering the server
 */
static int register_server(struct thread *server, u64 callback, u64 max_client,
			   u64 vm_config_ptr)
{
	int r;
	struct server_ipc_config *server_ipc_config;
	struct ipc_vm_config *vm_config;
	BUG_ON(server == NULL);

	// Create the server ipc_config
	server_ipc_config = kmalloc(sizeof(struct server_ipc_config));
	if (!server_ipc_config) {
		r = -ENOMEM;
		goto out_fail;
	}
	server->server_ipc_config = server_ipc_config;

	// Init the server ipc_config
	server_ipc_config->callback = callback;
	if (max_client > IPC_MAX_CONN_PER_SERVER) {
		r = -EINVAL;
		goto out_free_server_ipc_config;
	}
	server_ipc_config->max_client = max_client;
	server_ipc_config->conn_bmp =
	    kzalloc(BITS_TO_LONGS(max_client) * sizeof(long));
	if (!server_ipc_config->conn_bmp) {
		r = -ENOMEM;
		goto out_free_server_ipc_config;
	}
	// Get and check the parameter vm_config
	vm_config = &server_ipc_config->vm_config;
	r = copy_from_user((char *)vm_config, (char *)vm_config_ptr,
			   sizeof(*vm_config));
	if (r < 0)
		goto out_free_conn_bmp;
	if (!is_user_addr_range(vm_config->stack_base_addr,
				vm_config->stack_size) ||
	    !is_user_addr_range(vm_config->buf_base_addr,
				vm_config->buf_size) ||
	    !IS_ALIGNED(vm_config->stack_base_addr, PAGE_SIZE) ||
	    !IS_ALIGNED(vm_config->stack_size, PAGE_SIZE) ||
	    !IS_ALIGNED(vm_config->buf_base_addr, PAGE_SIZE) ||
	    !IS_ALIGNED(vm_config->buf_size, PAGE_SIZE)) {
		r = -EINVAL;
		goto out_free_conn_bmp;
	}

	return r;

 out_free_conn_bmp:
	kfree(server_ipc_config->conn_bmp);
 out_free_server_ipc_config:
	kfree(server_ipc_config);
 out_fail:
	return r;
}

u64 sys_register_server(u64 callback, u64 max_client, u64 vm_config_ptr)
{
	return register_server(current_thread, callback, max_client,
			       vm_config_ptr);
}
