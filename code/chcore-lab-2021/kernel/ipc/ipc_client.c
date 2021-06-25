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
 * Helper function called when an ipc_connection is created
 */
static struct thread *create_server_thread(struct thread *src)
{
	struct thread *new;

	new = kmalloc(sizeof(struct thread));
	BUG_ON(new == NULL);

	new->vmspace = obj_get(src->process, VMSPACE_OBJ_ID, TYPE_VMSPACE);
	BUG_ON(!new->vmspace);

	// Init the thread ctx
	new->thread_ctx = create_thread_ctx();
	if (!new->thread_ctx)
		goto out_fail;
	memcpy((char *)&(new->thread_ctx->ec),
	       (const char *)&(src->thread_ctx->ec), sizeof(arch_exec_cont_t));
	new->thread_ctx->prio = SHADOW_THREAD_PRIO;
	new->thread_ctx->state = TS_INIT;
	new->thread_ctx->affinity = NO_AFF;
	new->thread_ctx->sc = NULL;
	new->thread_ctx->type = TYPE_SHADOW;

	// Init the server ipc
	new->server_ipc_config = kzalloc(sizeof(struct server_ipc_config));
	if (!new->server_ipc_config)
		goto out_destroy_thread_ctx;
	new->server_ipc_config->callback = src->server_ipc_config->callback;
	new->server_ipc_config->vm_config = src->server_ipc_config->vm_config;
	new->process = src->process;

	obj_put(new->vmspace);
	return new;

 out_destroy_thread_ctx:
	destroy_thread_ctx(new);
 out_fail:
	obj_put(new->vmspace);
	kfree(new);
	return NULL;
}

/**
 * Helper function to create an ipc_connection by the client thread
 */
static int create_connection(struct thread *source, struct thread *target,
			     struct ipc_vm_config *client_vm_config)
{
	struct ipc_connection *conn = NULL;
	int ret = 0;
	int conn_cap = 0, server_conn_cap = 0;
	struct pmobject *stack_pmo, *buf_pmo;
	int conn_idx;
	struct server_ipc_config *server_ipc_config;
	struct ipc_vm_config *vm_config;
	u64 server_stack_base, server_buf_base, client_buf_base;
	u64 stack_size, buf_size;

	BUG_ON(source == NULL);
	BUG_ON(target == NULL);

	// Get the ipc_connection
	conn = obj_alloc(TYPE_CONNECTION, sizeof(*conn));
	if (!conn) {
		ret = -ENOMEM;
		goto out_fail;
	}
	conn->target = create_server_thread(target);
	if (!conn->target) {
		ret = -ENOMEM;
		goto out_fail;
	}
	// Get the server's ipc config
	server_ipc_config = target->server_ipc_config;
	vm_config = &server_ipc_config->vm_config;
	conn_idx = find_next_zero_bit(server_ipc_config->conn_bmp,
				      server_ipc_config->max_client, 0);
	set_bit(conn_idx, server_ipc_config->conn_bmp);

	// Create the server thread's stack
	server_stack_base =
	    vm_config->stack_base_addr + conn_idx * vm_config->stack_size;
	stack_size = vm_config->stack_size;
	kdebug("server stack base:%lx size:%lx\n", server_stack_base,
	       stack_size);
	stack_pmo = kmalloc(sizeof(struct pmobject));
	if (!stack_pmo) {
		ret = -ENOMEM;
		goto out_free_obj;
	}
	pmo_init(stack_pmo, PMO_DATA, stack_size, 0);
	vmspace_map_range(target->vmspace, server_stack_base, stack_size,
			  VMR_READ | VMR_WRITE, stack_pmo);

	conn->server_stack_top = server_stack_base + stack_size;

	// Create and map the shared buffer for client and server
	server_buf_base =
	    vm_config->buf_base_addr + conn_idx * vm_config->buf_size;
	client_buf_base = client_vm_config->buf_base_addr;
	buf_size = MIN(vm_config->buf_size, client_vm_config->buf_size);
	client_vm_config->buf_size = buf_size;
	kdebug("server buf base:%lx size:%lx, client base:%lx\n",
	       server_stack_base, stack_size, client_buf_base);

	buf_pmo = kmalloc(sizeof(struct pmobject));
	if (!buf_pmo) {
		ret = -ENOMEM;
		goto out_free_stack_pmo;
	}
	pmo_init(buf_pmo, PMO_DATA, buf_size, 0);

	vmspace_map_range(current_thread->vmspace, client_buf_base, buf_size,
			  VMR_READ | VMR_WRITE, buf_pmo);
	vmspace_map_range(target->vmspace, server_buf_base, buf_size,
			  VMR_READ | VMR_WRITE, buf_pmo);

	conn->buf.client_user_addr = client_buf_base;
	conn->buf.server_user_addr = server_buf_base;

	conn_cap = cap_alloc(current_process, conn, 0);
	if (conn_cap < 0) {
		ret = conn_cap;
		goto out_free_obj;
	}

	server_conn_cap =
	    cap_copy(current_process, target->process, conn_cap, 0, 0);
	if (server_conn_cap < 0) {
		ret = server_conn_cap;
		goto out_free_obj;
	}
	conn->server_conn_cap = server_conn_cap;

	return conn_cap;
 out_free_stack_pmo:
	kfree(stack_pmo);
 out_free_obj:
	obj_free(conn);
 out_fail:
	return ret;
}

u32 sys_register_client(u32 server_cap, u64 vm_config_ptr)
{
	struct thread *client = current_thread;
	struct thread *server = NULL;
	struct ipc_connection *conn;
	struct ipc_vm_config vm_config = { 0 };
	u64 client_buf_size;
	int conn_cap = 0;
	int r = 0;

	r = copy_from_user((char *)&vm_config, (char *)vm_config_ptr,
			   sizeof(vm_config));
	if (r < 0)
		goto out_fail;
	if (!is_user_addr_range(vm_config.buf_base_addr, vm_config.buf_size) ||
	    !IS_ALIGNED(vm_config.buf_base_addr, PAGE_SIZE) ||
	    !IS_ALIGNED(vm_config.buf_size, PAGE_SIZE)) {
		r = -EINVAL;
		goto out_fail;
	}

	server = obj_get(current_thread->process, server_cap, TYPE_THREAD);
	if (!server) {
		r = -ECAPBILITY;
		goto out_fail;
	}

	client_buf_size = vm_config.buf_size;
	conn_cap = create_connection(client, server, &vm_config);
	if (conn_cap < 0) {
		r = conn_cap;
		goto out_obj_put_thread;
	}

	conn = obj_get(current_process, conn_cap, TYPE_CONNECTION);

	if (client_buf_size != vm_config.buf_size) {
		r = copy_to_user((char *)vm_config_ptr, (char *)&vm_config,
				 sizeof(vm_config));
		if (r < 0)
			goto out_obj_put_conn;
	}

	r = conn_cap;
 out_obj_put_conn:
	obj_put(conn);
 out_obj_put_thread:
	obj_put(server);
 out_fail:
	return r;
}
