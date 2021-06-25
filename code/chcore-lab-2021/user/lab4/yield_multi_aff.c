#include <lib/print.h>
#include <lib/syscall.h>
#include <lib/thread.h>

#define PRIO 255
#define INFO 233

#define THREAD_NUM 4

volatile u64 start_flags[THREAD_NUM];
int child_thread_caps[THREAD_NUM];

void *thread_routine(void *arg)
{
	u32 times = 0;
	u64 thread_id = (u64) arg;
	u64 next_thread_id = (thread_id + 1) % THREAD_NUM;
	u64 prev_thread_id = (thread_id + THREAD_NUM - 1) % THREAD_NUM;
	int aff;

	while (times < 3) {
		times++;
		while (start_flags[thread_id] == 0) ;
		start_flags[thread_id] = 0;

		usys_yield();

		aff = usys_get_affinity(child_thread_caps[thread_id]);

		printf("Iteration %lu, thread %lu, cpu %u, aff %d\n", times,
		       thread_id, usys_get_cpu_id(), aff);

		usys_set_affinity(child_thread_caps[prev_thread_id],
				  (thread_id + times) % 4);

		start_flags[next_thread_id] = 1;

		usys_yield();
	}

	/* usys_exit: just de-schedule itself without reclaiming the resource */
	usys_exit(0);
	return 0;
}

int main(int argc, char *argv[])
{
	int i;
	u64 thread_i;

	for (thread_i = 0; thread_i < THREAD_NUM; ++thread_i) {
		start_flags[thread_i] = 0;
		child_thread_caps[thread_i] =
		    create_thread(thread_routine, thread_i, PRIO, thread_i % 4);
		if (child_thread_caps[thread_i] < 0)
			printf("Create thread failed, return %d\n",
			       child_thread_caps[thread_i]);
		for (i = 0; i < 10000; i++) ;
	}

	start_flags[0] = 1;

	return 0;
}
