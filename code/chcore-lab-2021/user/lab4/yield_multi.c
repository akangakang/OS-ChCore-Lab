#include <lib/print.h>
#include <lib/syscall.h>
#include <lib/thread.h>

#define PRIO 255
#define INFO 233

#define THREAD_NUM 4

volatile u64 start_flags[THREAD_NUM];

void *thread_routine(void *arg)
{
	u64 thread_id = (u64) arg;

	while (start_flags[thread_id] == 0) ;

	printf("Hello, I am thread %u on cpu %u\n", thread_id,
	       usys_get_cpu_id());

	start_flags[(thread_id + 1) % THREAD_NUM] = 1;

	/* usys_exit: just de-schedule itself without reclaiming the resource */
	usys_exit(0);
	return 0;
}

int main(int argc, char *argv[])
{
	int child_thread_cap;
	int i;
	u64 thread_i;

	for (thread_i = 0; thread_i < THREAD_NUM; ++thread_i) {
		start_flags[thread_i] = 0;
		child_thread_cap =
		    create_thread(thread_routine, thread_i, PRIO, thread_i % 4);
		if (child_thread_cap < 0)
			printf("Create thread failed, return %d\n",
			       child_thread_cap);
		for (i = 0; i < 10000; i++) ;
	}

	start_flags[0] = 1;

	return 0;
}
