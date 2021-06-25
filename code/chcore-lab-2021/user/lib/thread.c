#include <lib/syscall.h>
#include <lib/defs.h>
#include <lib/print.h>
#include <lib/type.h>

int thread_num_in_process = 0;

int create_thread(void *(*func) (void *), u64 arg, u32 prio, s32 cpuid)
{
	int child_stack_pmo_cap = 0;
	int child_thread_cap = 0;
	int ret = 0;

	child_stack_pmo_cap = usys_create_pmo(CHILD_THREAD_STACK_SIZE,
					      PMO_ANONYM);
	if (child_stack_pmo_cap < 0)
		return child_stack_pmo_cap;

	ret = usys_map_pmo(SELF_CAP,
			   child_stack_pmo_cap,
			   CHILD_THREAD_STACK_BASE +
			   thread_num_in_process * CHILD_THREAD_STACK_SIZE,
			   VM_READ | VM_WRITE);
	if (ret < 0)
		return ret;
	thread_num_in_process += 1;

	child_thread_cap = usys_create_thread(SELF_CAP, CHILD_THREAD_STACK_BASE
					      +
					      thread_num_in_process *
					      CHILD_THREAD_STACK_SIZE,
					      (u64) func, arg, prio, cpuid);
	return child_thread_cap;
}
