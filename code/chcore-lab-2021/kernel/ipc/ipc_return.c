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
 * Lab4 - exercise 16
 * Helper function
 * Server thread calls this function and then return to client thread
 * This function should never return
 *
 * Replace the place_holder to correct value!
 */
static int thread_migrate_to_client(struct ipc_connection *conn, u64 ret_value)
{
	struct thread *source = conn->source;
	current_thread->active_conn = NULL;

	/**
	 * Lab4 - exercise 16
	 * The return value returned by server thread;
	 */
	arch_set_thread_return(source, ret_value);
	/**
	 * Switch to the client
	 */
	switch_to_thread(source);
	eret_to_thread(switch_context());

	/* Function never return */
	BUG_ON(1);
	return 0;
}

/**
 * The thread of ipc_connection calls sys_ipc_return
 * you should migrate to the client now.
 * This function should never return!
 */
void sys_ipc_return(u64 ret)
{
	struct ipc_connection *conn = current_thread->active_conn;

	if (conn == NULL) {
		WARN("An inactive thread calls ipc_return\n");
		goto out;
	}

	thread_migrate_to_client(conn, ret);

	BUG("This function should never\n");
 out:
	return;
}
