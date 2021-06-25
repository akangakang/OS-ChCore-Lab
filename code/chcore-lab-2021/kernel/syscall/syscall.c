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

#include <common/types.h>
#include <common/uart.h>
#include <common/smp.h>
#include <common/uaccess.h>
#include <common/kmalloc.h>
#include <common/mm.h>
#include <common/kprint.h>
#include <common/fs.h>
#include "syscall_num.h"

void sys_debug(long arg)
{
	kinfo("[syscall] sys_debug: %lx\n", arg);
}

void sys_putc(char ch)
{
	/*
	 * Lab3: Your code here
	 * Send ch to the screen in anyway as your like
	 */
	uart_send(ch);
}

u32 sys_getc(void)
{
	return uart_recv();
}

/* 
 * Lab4 - exercise 9
 * Finish the sys_get_cpu_id syscall
 */
u32 sys_get_cpu_id(void)
{
	return smp_get_cpu_id();
}

/*
 * Lab3: Your code here
 * Update the syscall table as you like to redirect syscalls
 * to functions accordingly
 */
const void *syscall_table[NR_SYSCALL] = {
	// [0 ... NR_SYSCALL - 1] = sys_debug,
	/* lab3 syscalls finished */
	/* [BUG] : merge 的时候丢了这些 */
	[SYS_putc] = sys_putc,
	[SYS_exit] = sys_exit,
	[SYS_create_pmo] = sys_create_pmo,
	[SYS_map_pmo] = sys_map_pmo,
	[SYS_handle_brk] = sys_handle_brk,

	[SYS_getc] = sys_getc,
	[SYS_yield] = sys_yield,
	[SYS_create_device_pmo] = sys_create_device_pmo,
	[SYS_unmap_pmo] = sys_unmap_pmo,
	[SYS_create_thread] = sys_create_thread,
	[SYS_create_process] = sys_create_process,
	[SYS_register_server] = sys_register_server,
	[SYS_register_client] = sys_register_client,
	[SYS_ipc_call] = sys_ipc_call,
	[SYS_ipc_return] = sys_ipc_return,
	/* 
	 * Lab4 - exercise 17
	 * Add syscall
	 */
	[SYS_ipc_reg_call]=sys_ipc_reg_call,
	[SYS_cap_copy_to] = sys_cap_copy_to,
	[SYS_cap_copy_from] = sys_cap_copy_from,
	[SYS_set_affinity] = sys_set_affinity,
	[SYS_get_affinity] = sys_get_affinity,
	/* 
	 * Lab4 - exercise 9
	 * Add syscall
	 */
	[SYS_get_cpu_id] = sys_get_cpu_id,

	[SYS_create_pmos] = sys_create_pmos,
	[SYS_map_pmos] = sys_map_pmos,
	[SYS_write_pmo] = sys_write_pmo,
	[SYS_read_pmo] = sys_read_pmo,
	[SYS_transfer_caps] = sys_transfer_caps,

	/* TMP FS */
	[SYS_fs_load_cpio] = sys_fs_load_cpio,

	[SYS_debug] = sys_debug
};
