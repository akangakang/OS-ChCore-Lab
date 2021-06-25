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

#define NR_SYSCALL   256

void sys_exit(void);
void sys_create_pmo(void);
void sys_map_pmo(void);
void sys_handle_brk(void);
/* lab3 syscalls finished */

void sys_yield(void);
void sys_create_device_pmo(void);
void sys_create_thread(void);
void sys_create_process(void);
void sys_cap_copy_to(void);
void sys_cap_copy_from(void);
void sys_unmap_pmo(void);
void sys_set_affinity(void);
void sys_get_affinity(void);

void sys_create_pmos(void);
void sys_map_pmos(void);
void sys_write_pmo(void);
void sys_transfer_caps(void);
void sys_read_pmo(void);

void sys_register_server(void);
void sys_register_client(void);
void sys_ipc_call(void);
void sys_ipc_reg_call(void);
void sys_ipc_return(void);

#define SYS_putc				0
#define SYS_getc				1
#define SYS_yield				2
#define SYS_exit				3
#define SYS_sleep				4
#define SYS_create_pmo				5
#define SYS_map_pmo				6
#define SYS_create_thread			7
#define SYS_create_process			8
#define SYS_register_server			9
#define SYS_register_client			10
#define SYS_get_conn_stack			11
#define SYS_ipc_call				12
#define SYS_ipc_return				13
#define SYS_cap_copy_to				15
#define SYS_cap_copy_from			16
#define SYS_unmap_pmo				17
#define SYS_set_affinity                        18
#define SYS_get_affinity                        19
#define SYS_create_device_pmo			20

/* Lab4 specfic */
#define SYS_get_cpu_id                          50
#define SYS_ipc_reg_call                        51

#define SYS_create_pmos                         101
#define SYS_map_pmos                            102
#define SYS_write_pmo                           103
#define SYS_read_pmo				104
#define SYS_transfer_caps                       105

#define SYS_handle_brk				201

#define SYS_fs_load_cpio			253
#define SYS_debug			        255
