#pragma once

#include <lib/type.h>
typedef struct ipc_struct {
	u64 conn_cap;
	u64 shared_buf;
	u64 shared_buf_len;
} ipc_struct_t;

typedef struct ipc_msg {
	u64 server_conn_cap;
	u64 data_len;
	u64 cap_slot_number;
	u64 data_offset;
	u64 cap_slots_offset;
} ipc_msg_t;

struct ipc_vm_config {
	u64 stack_base_addr;
	u64 stack_size;
	u64 buf_base_addr;
	u64 buf_size;
};

int ipc_register_client(int server_thread_cap, ipc_struct_t * ipc_struct);
ipc_msg_t *ipc_create_msg(ipc_struct_t * icb, u64 data_len,
			  u64 cap_slot_number);
char *ipc_get_msg_data(ipc_msg_t * ipc_msg);
u64 ipc_get_msg_cap(ipc_msg_t * ipc_msg, u64 cap_id);
int ipc_set_msg_data(ipc_msg_t * ipc_msg, char *data, u64 offset, u64 len);
int ipc_set_msg_cap(ipc_msg_t * ipc_msg, u64 cap_slot_index, u32 cap);
int ipc_destroy_msg(ipc_msg_t * ipc_msg);

int ipc_call(ipc_struct_t * icb, ipc_msg_t * ipc_msg);
int ipc_reg_call(ipc_struct_t * icb, u64 arg);
void ipc_return(int ret);

typedef void (*server_handler) (ipc_msg_t * ipc_msg);
int ipc_register_server(server_handler server_handler);

#define INFO_PAGE_VADDR ((void *)0x100000ll)
