#pragma once

#include <lib/type.h>

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

#define SYS_top                                 252
#define SYS_fs_load_cpio			253
#define SYS_debug			        255

int usys_fs_load_cpio(u64 vaddr);
/* TEMP END */

void usys_putc(char ch);
void usys_exit(int ret);
int usys_create_pmo(u64 size, u64 type);
int usys_map_pmo(u64 process_cap, u64 pmo_cap, u64 addr, u64 perm);
u64 usys_handle_brk(u64 addr);
/* lab3 syscalls finished */

u32 usys_getc(void);
u64 usys_yield(void);
int usys_create_device_pmo(u64 paddr, u64 size);
int usys_create_thread(u64 process_cap, u64 stack, u64 pc, u64 arg, u32 prio,
		       s32 cpuid);
int usys_create_process(void);
u64 usys_register_server(u64 callback, u64 max_client, u64 vm_config_ptr);
u32 usys_register_client(u32 server_cap, u64 vm_config_ptr);
u64 usys_ipc_call(u32 conn_cap, u64 arg0);
u64 usys_ipc_reg_call(u32 conn_cap, u64 arg0);
void usys_ipc_return(u64 ret);
int usys_debug(void);
int usys_cap_copy_to(u64 dest_process_cap, u64 src_slot_id);
int usys_cap_copy_from(u64 src_process_cap, u64 src_slot_id);
int usys_unmap_pmo(u64 process_cap, u64 pmo_cap, u64 addr);
int usys_set_affinity(u64 thread_cap, s32 aff);
s32 usys_get_affinity(u64 thread_cap);
u32 usys_get_cpu_id(void);

int usys_create_pmos(void *, u64);
int usys_map_pmos(u64, void *, u64);
int usys_write_pmo(u64, u64, void *, u64);
int usys_read_pmo(u64 cap, u64 offset, void *buf, u64 size);
int usys_transfer_caps(u64, int *, int, int *);

void usys_top(void);
