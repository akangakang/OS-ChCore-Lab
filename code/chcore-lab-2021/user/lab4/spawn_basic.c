
#include <lib/bug.h>
#include <lib/defs.h>
#include <lib/proc.h>
#include <lib/spawn.h>
#include <lib/syscall.h>
#include <lib/type.h>

#define CHILD_INFO_VADDR 0xb0000000	/* 2M */

int main(int argc, char *argv[], char *envp[])
{
	int ret = 0;

	usys_fs_load_cpio(CPIO_BIN);

	/* create a new process */
	printf("[Parent] create the child process.\n");

	ret = spawn("/spawn_child.bin", NULL, NULL, NULL, 0, NULL, 0, 1);
	fail_cond(ret < 0, "create_process returns %d\n", ret);

	return 0;
}
