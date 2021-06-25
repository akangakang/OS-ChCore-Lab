#include <lib/bug.h>
#include <lib/ipc.h>
#include <lib/launcher.h>
#include <lib/syscall.h>
#include <lib/type.h>

void ipc_dispatcher(ipc_msg_t * ipc_msg)
{
	ipc_return((u64) ipc_msg);
}

int main(int argc, char *argv[], char *envp[])
{
	int ret;
	void *info_page_addr;
	struct info_page *info_page;

	info_page_addr = (void *)(envp[0]);
	fail_cond(info_page_addr == NULL, "[Server] no info received.\n");

	ret = ipc_register_server(ipc_dispatcher);
	fail_cond(ret < 0, "[IPC Server] register server failed\n", ret);

	info_page = (struct info_page *)info_page_addr;
	info_page->ready_flag = 1;

	while (info_page->exit_flag != 1) {
		usys_yield();
	}

	printf("[Server] exit\n");
	return 0;
}
