#include <lib/syscall.h>
#include <lib/ipc.h>
#include <lib/defs.h>
#include <lib/string.h>
#include <lib/print.h>

ipc_msg_t *ipc_create_msg(ipc_struct_t * icb, u64 data_len, u64 cap_slot_number)
{
	ipc_msg_t *ipc_msg;
	int i;

	ipc_msg = (ipc_msg_t *) icb->shared_buf;
	ipc_msg->data_len = data_len;
	ipc_msg->cap_slot_number = cap_slot_number;

	ipc_msg->data_offset = sizeof(*ipc_msg);
	ipc_msg->cap_slots_offset = ipc_msg->data_offset + data_len;
	memset(ipc_get_msg_data(ipc_msg), 0, data_len);
	for (i = 0; i < cap_slot_number; i++)
		ipc_set_msg_cap(ipc_msg, i, -1);

	return ipc_msg;
}

char *ipc_get_msg_data(ipc_msg_t * ipc_msg)
{
	return (char *)ipc_msg + ipc_msg->data_offset;
}

int ipc_set_msg_data(ipc_msg_t * ipc_msg, char *data, u64 offset, u64 len)
{
	if (offset + len < offset || offset + len > ipc_msg->data_len)
		return -1;

	memcpy(ipc_get_msg_data(ipc_msg) + offset, data, len);
	return 0;
}

static u64 *ipc_get_msg_cap_ptr(ipc_msg_t * ipc_msg, u64 cap_id)
{
	return (u64 *) ((char *)ipc_msg + ipc_msg->cap_slots_offset) + cap_id;
}

u64 ipc_get_msg_cap(ipc_msg_t * ipc_msg, u64 cap_slot_index)
{
	if (cap_slot_index >= ipc_msg->cap_slot_number)
		return -1;
	return *ipc_get_msg_cap_ptr(ipc_msg, cap_slot_index);
}

int ipc_set_msg_cap(ipc_msg_t * ipc_msg, u64 cap_slot_index, u32 cap)
{
	if (cap_slot_index >= ipc_msg->cap_slot_number)
		return -1;
	*ipc_get_msg_cap_ptr(ipc_msg, cap_slot_index) = cap;
	return 0;
}

/* FIXME: currently ipc_msg is not dynamically allocated so that no need to free */
int ipc_destroy_msg(ipc_msg_t * ipc_msg)
{
	return 0;
}

#define SERVER_STACK_BASE	0x7000000
#define SERVER_STACK_SIZE	0x1000
#define SERVER_BUF_BASE		0x7400000
#define SERVER_BUF_SIZE		0x1000
#define CLIENT_BUF_BASE		0x7800000
#define CLIENT_BUF_SIZE		0x1000
#define MAX_CLIENT		16

int ipc_register_server(server_handler server_handler)
{
	struct ipc_vm_config vm_config = {
		.stack_base_addr = SERVER_STACK_BASE,
		.stack_size = SERVER_STACK_SIZE,
		.buf_base_addr = SERVER_BUF_BASE,
		.buf_size = SERVER_BUF_SIZE,
	};
	return usys_register_server((u64) server_handler, MAX_CLIENT,
				    (u64) & vm_config);
}

int ipc_register_client(int server_thread_cap, ipc_struct_t * ipc_struct)
{
	int conn_cap;

	struct ipc_vm_config vm_config = {
		.buf_base_addr = CLIENT_BUF_BASE,
		.buf_size = CLIENT_BUF_SIZE,
	};

	conn_cap = usys_register_client((u32) server_thread_cap,
					(u64) & vm_config);

	if (conn_cap < 0)
		return -1;
	ipc_struct->shared_buf = vm_config.buf_base_addr;
	ipc_struct->shared_buf_len = vm_config.buf_size;
	ipc_struct->conn_cap = conn_cap;

	return 0;
}

int ipc_call(ipc_struct_t * icb, ipc_msg_t * ipc_msg)
{
	u64 ret = 0;
	ret = usys_ipc_call(icb->conn_cap, (u64) ipc_msg);

	return ret;
}

int ipc_reg_call(ipc_struct_t * icb, u64 arg)
{
	u64 ret = 0;
	ret = usys_ipc_reg_call(icb->conn_cap, (u64) arg);

	return ret;
}

void ipc_return(int ret)
{
	usys_ipc_return((u64) ret);
}
