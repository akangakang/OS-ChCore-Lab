#include <common/smp.h>
#include <common/kprint.h>
#include <common/macro.h>
#include <common/kmalloc.h>
#include <tests/tests.h>

void init_test(void)
{
	u32 ret = 0;

	global_barrier_init();
	ret = lock_init(&test_lock);
	BUG_ON(ret != 0);
}

void run_test(bool is_bsp)
{
	if (is_bsp)
		kinfo("[ChCore] kernel tests\n");

	tst_mutex(is_bsp);
	tst_big_lock(is_bsp);

	tst_sched_cooperative(is_bsp);
	tst_sched_preemptive(is_bsp);
	tst_sched_affinity(is_bsp);
	tst_sched(is_bsp);

	if (is_bsp) {
		kinfo("[ChCore] pass all kernel tests\n");
	}
}
