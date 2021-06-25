
#include <lib/bug.h>
#include <lib/syscall.h>

int main()
{
	int ret = 0;
	int aff = 3;

	printf("Main thread on cpu %u\n", usys_get_cpu_id());

	ret = usys_set_affinity(-1, aff);
	fail_cond(ret != 0, "Set affinity failed\n");

	printf("Main thread set affinity %d\n", aff);

	usys_yield();

	ret = usys_get_affinity(-1);

	printf("Main thread affinity %d\n", ret);

	printf("Main thread exits on cpu_id: %u\n", usys_get_cpu_id());

	return 0;
}
