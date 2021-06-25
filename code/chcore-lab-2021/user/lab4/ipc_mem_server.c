#include <lib/bug.h>
#include <lib/defs.h>
#include <lib/ipc.h>
#include <lib/launcher.h>
#include <lib/syscall.h>

#define SHARED_PAGE_VADDR 0x20000000

void ipc_dispatcher(ipc_msg_t * ipc_msg)
{
	int ret = 0;

	fail_cond(ipc_msg->data_len != 0, "data_len %lu\n", ipc_msg->data_len);
	fail_cond(ipc_msg->cap_slot_number != 1, "cap_slot_number 5lu\n",
		  ipc_msg->cap_slot_number);
	int cap = ipc_get_msg_cap(ipc_msg, 0);

	/* map copied pmo to another va */
	ret = usys_map_pmo(SELF_CAP, cap, SHARED_PAGE_VADDR + PAGE_SIZE,
			   VM_READ | VM_WRITE);
	fail_cond(ret < 0, "usys_map_pmo on copied pmo ret %d\n", ret);

	/* read from shared memory should be MAGIC_NUM */
	printf("[Server] read %x\n", *(int *)(SHARED_PAGE_VADDR + PAGE_SIZE));
	ret = 0;

	ipc_return(ret);
}

int main(int argc, char *argv[], char *envp[])
{
	int ret;
	void *info_page_addr;
	struct info_page *info_page;

	info_page_addr = (void *)(envp[0]);
	fail_cond(info_page_addr == NULL, "[Server] no info received.\n");

	ret = ipc_register_server(ipc_dispatcher);
	fail_cond(ret < 0, "[Server] register server failed\n", ret);

	info_page = (struct info_page *)info_page_addr;
	info_page->ready_flag = 1;

	while (info_page->exit_flag != 1) {
		usys_yield();
	}

	printf("[Server] exit\n");
	return 0;
}
