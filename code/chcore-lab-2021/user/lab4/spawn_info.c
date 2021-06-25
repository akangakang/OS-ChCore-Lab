
#include <lib/bug.h>
#include <lib/defs.h>
#include <lib/proc.h>
#include <lib/spawn.h>
#include <lib/syscall.h>
#include <lib/type.h>

#define CHILD_INFO_VADDR 0xb0000000	/* 2M */

int main(int argc, char *argv[], char *envp[])
{
	int i, ret = 0;
	int info_pmo_cap;
	int new_process_cap, new_thread_cap;
	struct info_page *info_page;
	struct pmo_map_request pmo_map_reqs[1];
	char *msg = "The spawn() seems ok?";

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
	info_page->nr_args = 22;
	for (i = 0; i < info_page->nr_args; i++) {
		info_page->args[i] = (u64) (msg[i]);
	}

	pmo_map_reqs[0].pmo_cap = info_pmo_cap;
	pmo_map_reqs[0].addr = 0x100000000;
	pmo_map_reqs[0].perm = VM_READ | VM_WRITE;

	/* create a new process */

	ret = spawn("/spawn_child.bin", &new_process_cap, &new_thread_cap,
		    pmo_map_reqs, 1, &info_pmo_cap, 1, 2);
	fail_cond(ret < 0, "create_process returns %d\n", ret);

	printf("[Parent] create the child process with cap %d.\n",
	       new_process_cap);

	while (info_page->ready_flag != 1)
		usys_yield();

	printf("[Parent] Are you ok...\n");
	printf("[Parent] Bye\n");

	info_page->exit_flag = 1;

	return 0;
}
