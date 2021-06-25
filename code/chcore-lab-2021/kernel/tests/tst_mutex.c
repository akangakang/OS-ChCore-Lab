#include <common/smp.h>
#include <common/kprint.h>
#include <common/macro.h>
#include <common/kmalloc.h>

#include <tests/tests.h>

#define LOCK_TEST_NUM 100000

/* Mutex test count */
struct lock test_lock;
unsigned long mutex_test_count = 0;
unsigned long big_lock_test_count = 0;

void tst_mutex(bool is_bsp)
{
	global_barrier(is_bsp);

	/* Mutex Lock */
	for (int i = 0; i < LOCK_TEST_NUM; i++) {
		if (i % 2)
			while (try_lock(&test_lock) != 0) ;
		else
			lock(&test_lock);
		/* Critical Section */
		mutex_test_count++;
		unlock(&test_lock);
	}

	global_barrier(is_bsp);
	BUG_ON(mutex_test_count != PLAT_CPU_NUM * LOCK_TEST_NUM);
	global_barrier(is_bsp);
	if (is_bsp) {
		printk("pass tst_mutex\n");
	}
}

void tst_big_lock(bool is_bsp)
{
	int i;

	if (is_bsp) {
		big_lock_test_count = 0;
		BUG_ON(!is_locked(&big_kernel_lock));
		unlock_kernel();
	}
	// kinfo("CPU%u 1-1\n", cpu_id);
	global_barrier(is_bsp);

	for (i = 0; i < LOCK_TEST_NUM; ++i) {
		if (i % 2)
			while (try_lock(&big_kernel_lock) != 0) ;
		else
			lock_kernel();
		big_lock_test_count += 1;
		unlock_kernel();
		// if (i % 100 == 1)
		//      kinfo("pri:%d\n", i);
	}

	// kinfo("CPU%u 1-2\n", cpu_id);
	global_barrier(is_bsp);
	BUG_ON(LOCK_TEST_NUM * PLAT_CPU_NUM != big_lock_test_count);
	global_barrier(is_bsp);
	if (is_bsp) {
		lock_kernel();
		printk("pass tst_big_lock\n");
	}
}
