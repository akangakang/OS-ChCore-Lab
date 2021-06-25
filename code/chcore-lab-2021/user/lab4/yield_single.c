#include <lib/print.h>
#include <lib/syscall.h>
#include <lib/thread.h>

#define PRIO 255
#define INFO 233

void *thread_routine(void *arg)
{
	u32 times = 0;
	u64 thread_id = (u64) arg;
	
	printf("Hello, I am thread %u\n", thread_id);
	usys_yield();

	while (times++ < 10) {
		printf("Iteration %lu, thread %lu, cpu %u\n", times, thread_id,
		       usys_get_cpu_id());
		usys_yield();
	}
	/* usys_exit: just de-schedule itself without reclaiming the resource */
	usys_exit(0);
	return 0;
}

int main(int argc, char *argv[])
{
	int child_thread_cap;
	int i;
	u64 thread_i;
	
	// kinfo("!!!\n");
	for (thread_i = 0; thread_i < 2; ++thread_i) {
		child_thread_cap =
		    create_thread(thread_routine, thread_i, PRIO, 0);
		if (child_thread_cap < 0)
			printf("Create thread failed, return %d\n",
			       child_thread_cap);
		for (i = 0; i < 10000; i++) ;
	}

	return 0;
}
