
#include <lib/bug.h>
#include <lib/defs.h>
#include <lib/ipc.h>
#include <lib/print.h>
#include <lib/proc.h>
#include <lib/spawn.h>
#include <lib/syscall.h>
#include <lib/type.h>

#define MAGIC_NUM 0xdeadbeef
#define SHARED_PAGE_VADDR 0xa00000000
#define CHILD_INFO_VADDR 0xb0000000	/* 2M */

int main(int argc, char *argv[], char *envp[])
{
	int ret = 0;
	int shared_page_pmo_cap;
	int info_pmo_cap;
	int new_process_cap, new_thread_cap;
	ipc_struct_t client_ipc_struct;
	ipc_msg_t *ipc_msg;
	struct info_page *info_page;
	struct pmo_map_request pmo_map_reqs[1];

	usys_fs_load_cpio(CPIO_BIN);

	/* prepare the info_page (transfer init info) for the new process */
	info_pmo_cap = usys_create_pmo(PAGE_SIZE, PMO_DATA);
	fail_cond(info_pmo_cap < 0, "usys_create_ret ret %d\n", ret);

	ret = usys_map_pmo(SELF_CAP, info_pmo_cap, CHILD_INFO_VADDR,
			   VM_READ | VM_WRITE);
	fail_cond(ret < 0, "usys_map_pmo ret %d\n", ret);

	info_page = (void *)CHILD_INFO_VADDR;
	info_page->ready_flag = 0;
	info_page->exit_flag = 0;
	info_page->nr_args = 0;

	pmo_map_reqs[0].pmo_cap = info_pmo_cap;
	pmo_map_reqs[0].addr = 0x100000000;
	pmo_map_reqs[0].perm = VM_READ | VM_WRITE;

	/* create a new process */
	printf("[Parent] create the server process.\n");

	ret = spawn("/ipc_mem_server.bin", &new_process_cap, &new_thread_cap,
		    pmo_map_reqs, 1, NULL, 0, 1);
	fail_cond(ret < 0, "create_process returns %d\n", ret);

	while (info_page->ready_flag != 1)
		usys_yield();

	/* register IPC client */
	ret = ipc_register_client(new_thread_cap, &client_ipc_struct);
	fail_cond(ret < 0, "ipc_register_client failed\n");

	/* create share memory */
	shared_page_pmo_cap = usys_create_pmo(PAGE_SIZE, PMO_DATA);
	fail_cond(shared_page_pmo_cap < 0, "usys_create_ret ret %d\n", ret);

	/* map shared memory */
	ret = usys_map_pmo(SELF_CAP, shared_page_pmo_cap, SHARED_PAGE_VADDR,
			   VM_READ | VM_WRITE);
	fail_cond(ret < 0, "usys_map_pmo ret %d\n", ret);

	/* write shared memory */
	*(int *)SHARED_PAGE_VADDR = MAGIC_NUM;
	/* IPC send cap */
	ipc_msg = ipc_create_msg(&client_ipc_struct, 0, 1);
	ipc_set_msg_cap(ipc_msg, 0, shared_page_pmo_cap);
	ipc_call(&client_ipc_struct, ipc_msg);
	ipc_destroy_msg(ipc_msg);

	printf("[Client] exit\n");
	info_page->exit_flag = 1;

	return 0;
}
