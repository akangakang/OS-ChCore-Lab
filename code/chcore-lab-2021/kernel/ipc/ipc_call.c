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
#include <exception/exception.h>
#include <common/kmalloc.h>
#include <common/mm.h>
#include <common/uaccess.h>
#include <process/thread.h>
#include <sched/context.h>
#include <sched/sched.h>

/**
 * A helper function to transfer all the ipc_msg's capbilities of client's
 * process to server's process
 */
#define MAX_CAP_TRANSFER 8
int ipc_send_cap(struct ipc_connection *conn, ipc_msg_t *ipc_msg)
{
	int i, r;
	u64 cap_slot_number;
	u64 cap_slots_offset;
	u64 *cap_buf;

	r = copy_from_user((char *)&cap_slot_number,
					   (char *)&ipc_msg->cap_slot_number,
					   sizeof(cap_slot_number));
	if (r < 0)
		goto out;
	if (likely(cap_slot_number == 0))
	{
		r = 0;
		goto out;
	}
	else if (cap_slot_number >= MAX_CAP_TRANSFER)
	{
		r = -EINVAL;
		goto out;
	}

	r = copy_from_user((char *)&cap_slots_offset,
					   (char *)&ipc_msg->cap_slots_offset,
					   sizeof(cap_slots_offset));
	if (r < 0)
		goto out;

	cap_buf = kmalloc(cap_slot_number * sizeof(*cap_buf));
	if (!cap_buf)
	{
		r = -ENOMEM;
		goto out;
	}

	r = copy_from_user((char *)cap_buf, (char *)ipc_msg + cap_slots_offset,
					   sizeof(*cap_buf) * cap_slot_number);
	if (r < 0)
		goto out;

	for (i = 0; i < cap_slot_number; i++)
	{
		u64 dest_cap;

		kdebug("[IPC] send cap:%d\n", cap_buf[i]);
		dest_cap = cap_copy(current_process, conn->target->process,
							cap_buf[i], false, 0);
		if (dest_cap < 0)
			goto out_free_cap;
		cap_buf[i] = dest_cap;
	}

	r = copy_to_user((char *)ipc_msg + cap_slots_offset, (char *)cap_buf,
					 sizeof(*cap_buf) * cap_slot_number);
	if (r < 0)
		goto out_free_cap;

	kfree(cap_buf);
	return 0;

out_free_cap:
	for (--i; i >= 0; i--)
		cap_free(conn->target->process, cap_buf[i]);
	kfree(cap_buf);
out:
	return r;
}

/**
 * Lab4 - exercise 16
 * Helper function
 * Client thread calls this function and then return to server thread
 * This function should never return
 *
 * Replace the place_holder to correct value!
 */
static u64 thread_migrate_to_server(struct ipc_connection *conn, u64 arg)
{
	struct thread *target = conn->target;

	conn->source = current_thread;
	target->active_conn = conn;
	current_thread->thread_ctx->state = TS_WAITING;
	obj_put(conn);

	/**
	 * Lab4 - exercise 16
	 * This command set the sp register, read the file to find which field
	 * of the ipc_connection stores the stack of the server thread?
	 * */
	/*
	 * 这个stack是sp哦所以是栈顶
	 */
	arch_set_thread_stack(target, conn->server_stack_top);
	/**
	 * Lab4 - exercise 16
	 * This command set the ip register, read the file to find which field
	 * of the ipc_connection stores the instruction to be called when switch
	 * to the server?
	 * */
	arch_set_thread_next_ip(target, conn->target->server_ipc_config->callback);
	/**
	 * Lab4 - exercise 16
	 * The argument set by sys_ipc_call;
	 */
	arch_set_thread_arg(target, arg);

	/**
	 * Passing the scheduling context of the current thread to thread of
	 * connection
	 */
	target->thread_ctx->sc = current_thread->thread_ctx->sc;

	/**
	 * Switch to the server
	 */
	switch_to_thread(target);
	eret_to_thread(switch_context());

	/* Function never return */
	BUG_ON(1);
	return 0;
}

/**
 * Lab4 - exercise 16
 * The client thread calls sys_ipc_call to migrate to the server thread.
 * When you transfer the ipc_msg (which is the virtual address in the client
 * vmspace), do not forget to change the virtual address to server's vmspace.
 * This function should never return!
 * 
 * ipc_msg实际存储于客户端和服务器之间的每个ipc_connection独有的共享缓冲区起始地址上
 */
u64 sys_ipc_call(u32 conn_cap, ipc_msg_t *ipc_msg)
{
	struct ipc_connection *conn = NULL;
	u64 arg;
	int r;

	conn = obj_get(current_thread->process, conn_cap, TYPE_CONNECTION);
	if (!conn)
	{
		r = -ECAPBILITY;
		goto out_fail;
	}

	/**
	 * Lab4 - exercise 16
	 * Here, you need to transfer all the capbiliies of client thread to
	 * capbilities in server thread in the ipc_msg.
	 */

	r = copy_to_user((char *)&ipc_msg->server_conn_cap,
					 (char *)&conn->server_conn_cap, sizeof(u64));
	if (r < 0)
		goto out_obj_put;

	r = ipc_send_cap(conn, ipc_msg);
	if (r < 0)
		goto out_obj_put;

	/**
	 * Lab4 - exercise 16
	 * The arg is actually the 64-bit arg for ipc_dispatcher
	 * Then what value should the arg be?
	 * */

	/*
	 * 这个参数是传给ipc_dispatch的参数
	 * 是共享内存在server的虚拟地址
	 */
	arg = conn->buf.server_user_addr;
	thread_migrate_to_server(conn, arg);

	BUG("This function should never\n");
out_obj_put:
	obj_put(conn);
out_fail:
	return r;
}

/**
 * Lab4  - exercise 17
 * Implement your sys_ipc_reg_call
 * 记得在syscall.c里加上这个系统调用
 * */
u64 sys_ipc_reg_call(u32 conn_cap, u64 arg0)
{
	struct ipc_connection *conn = NULL;
	int r;

	conn = obj_get(current_thread->process, conn_cap, TYPE_CONNECTION);
	if (!conn)
	{
		return -ECAPBILITY;
		
	}

	thread_migrate_to_server(conn, arg0);

	BUG("sys_ipc_reg_call should never reach here\n");

	obj_put(conn);

	return r;
}
