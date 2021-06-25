#include <lib/bug.h>
#include <lib/defs.h>
#include <lib/launcher.h>
#include <lib/syscall.h>
#include <lib/type.h>

#define SHARED_PAGE_VADDR 0x20000000

int main(int argc, char *argv[], char *envp[])
{
	int i, ret;
	int transfered_cap;
	void *info_page_addr;
	struct info_page *info_page1, *info_page2;

	printf("[Child] Child on cpu %u\n", usys_get_cpu_id());
	printf("[Child] argv: 0x%lx, argv[0]: 0x%lx\n", (u64) argv,
	       (u64) argv[0]);
	info_page_addr = (void *)(envp[0]);
	printf("[Child] info_page_addr: 0x%lx\n", (u64) info_page_addr);
	transfered_cap = (int)((u64) envp[1]);
	printf("[Child] transfered_cap: %d\n", transfered_cap);

	if (info_page_addr) {
		info_page1 = (struct info_page *)info_page_addr;
		printf("[Child] ");
		for (i = 0; i < info_page1->nr_args - 1; i++) {
			printf("%c", (char)info_page1->args[i]);
			info_page1->args[i]++;
		}
		printf("\n");

		ret = usys_map_pmo(SELF_CAP, transfered_cap,
				   SHARED_PAGE_VADDR + PAGE_SIZE,
				   VM_READ | VM_WRITE);
		fail_cond(ret < 0, "usys_map_pmo on copied pmo ret %d\n", ret);

		info_page2 =
		    (struct info_page *)(SHARED_PAGE_VADDR + PAGE_SIZE);
		printf("[Child] ");
		for (i = 0; i < info_page2->nr_args - 1; i++) {
			printf("%c", (char)info_page2->args[i]);
		}
		printf("\n");

		ret = usys_unmap_pmo(SELF_CAP, transfered_cap,
				     SHARED_PAGE_VADDR + PAGE_SIZE);
		fail_cond(ret < 0, "usys_unmap_pmo on copied pmo ret %d\n",
			  ret);

		printf("[Child] Bye\n");
		info_page1->ready_flag = 1;

		while (info_page1->exit_flag != 1) {
			usys_yield();
		}
	} else {
		printf("[Child] Bye\n");
	}
	return 0;
}
