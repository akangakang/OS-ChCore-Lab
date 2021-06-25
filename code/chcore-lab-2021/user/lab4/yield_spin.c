#include <lib/print.h>
#include <lib/syscall.h>
#include <lib/thread.h>

#define PRIO 255
#define INFO 233

bool running = true;

void *thread_routine(void *arg)
{
	u64 thread_id = (u64) arg;

	printf("Hello, I am thread %u\n", thread_id);

	while (running) {

	}
	/* usys_exit: just de-schedule itself without reclaiming the resource */
	usys_exit(0);
	return 0;
}

int main(int argc, char *argv[])
{
	int child_thread_cap;

	child_thread_cap = create_thread(thread_routine, 0, PRIO, 0);
	if (child_thread_cap < 0)
		printf("Create thread failed, return %d\n", child_thread_cap);

	usys_yield();

	printf("Successfully regain the control!\n");

	return 0;
}
