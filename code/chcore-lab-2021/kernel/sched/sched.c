/*
 * Copyright (c) 2020 Institute of Parallel And Distributed Systems (IPADS), Shanghai Jiao Tong University (SJTU)
 * OS-Lab-2020 (i.e., ChCore) is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *   http://license.coscl.org.cn/MulanPSL
 *   THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 *   IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 *   PURPOSE.
 *   See the Mulan PSL v1 for more details.
 */

/* Scheduler related functions are implemented here */
#include <sched/sched.h>
#include <common/smp.h>
#include <common/kprint.h>
#include <common/machine.h>
#include <common/kmalloc.h>
#include <common/list.h>
#include <common/util.h>
#include <process/thread.h>
#include <common/macro.h>
#include <common/errno.h>
#include <process/thread.h>
#include <exception/exception.h>
#include <sched/context.h>

struct thread *current_threads[PLAT_CPU_NUM];

/* Chosen Scheduling Policies */
struct sched_ops *cur_sched_ops;

char thread_type[][TYPE_STR_LEN] = {
	"IDLE  ",
	"ROOT  ",
	"USER  ",
	"SHADOW",
	"KERNEL",
	"TESTS "
};

char thread_state[][STATE_STR_LEN] = {
	"TS_INIT      ",
	"TS_READY     ",
	"TS_INTER     ",
	"TS_RUNNING   ",
	"TS_EXIT      ",
	"TS_WAITING   ",
	"TS_EXITING   "
};

void arch_idle_ctx_init(struct thread_ctx *idle_ctx, void (*func) (void))
{
	/* Initialize to run the function `idle_thread_routine` */
	int i = 0;
	arch_exec_cont_t *ec = &(idle_ctx->ec);

	/* X0-X30 all zero */
	for (i = 0; i < REG_NUM; i++)
		ec->reg[i] = 0;
	/* SPSR_EL1 => Exit to EL1 */
	ec->reg[SPSR_EL1] = SPSR_EL1_KERNEL;
	/* ELR_EL1 => Next PC */
	ec->reg[ELR_EL1] = (u64) func;
}

void print_thread(struct thread *thread)
{
	printk
	    ("Thread %p\tType: %s\tState: %s\tCPU %d\tAFF %d\tBudget %d\tPrio: %d\t\n",
	     thread, thread_type[thread->thread_ctx->type],
	     thread_state[thread->thread_ctx->state], thread->thread_ctx->cpuid,
	     thread->thread_ctx->affinity, thread->thread_ctx->sc->budget,
	     thread->thread_ctx->prio);
}

int sched_is_running(struct thread *target)
{
	BUG_ON(!target);
	BUG_ON(!target->thread_ctx);

	if (target->thread_ctx->state == TS_RUNNING)
		return 1;
	return 0;
}

/*
 * Switch Thread to the specified one.
 * Set the correct thread state to running and the current_thread
 */
int switch_to_thread(struct thread *target)
{
	BUG_ON(!target);
	BUG_ON(!target->thread_ctx);
	BUG_ON((target->thread_ctx->state == TS_READY));

	target->thread_ctx->cpuid = smp_get_cpu_id();
	target->thread_ctx->state = TS_RUNNING;
	smp_wmb();
	current_thread = target;

	return 0;
}

/*
 * Switch vmspace and arch-related stuff
 * Return the context pointer which should be set to stack pointer register
 */
u64 switch_context(void)
{
	struct thread *target_thread;
	struct thread_ctx *target_ctx;

	target_thread = current_thread;		// want to change to *current_thread*
	BUG_ON(!target_thread);
	BUG_ON(!target_thread->thread_ctx);

	target_ctx = target_thread->thread_ctx;

	/* These 3 types of thread do not have vmspace */
	if (target_thread->thread_ctx->type != TYPE_IDLE &&
	    target_thread->thread_ctx->type != TYPE_KERNEL &&
	    target_thread->thread_ctx->type != TYPE_TESTS) {
		BUG_ON(!target_thread->vmspace);
		switch_thread_vmspace_to(target_thread);
	}
	/*
	 * Lab3: Your code here
	 * Return the correct value in order to make eret_to_thread work correctly
	 * in main.c
	 */

	/* eret_to_thread need *sp* as param */
	/*
	 * 返回 指向reg数组第一个元素的指针
	 * 即 thread_ctx 的起始地址
	 * 将该地址作为目标线程的内核栈顶
	 */
	
	return (u64)target_ctx->ec.reg;	
}

/* SYSCALL functions */

/**
 * Lab4 - exercise 9
 * Finish the sys_yield function
 */
void sys_yield(void)
{
	if(current_thread!=NULL && current_thread->thread_ctx!=NULL)
		current_thread->thread_ctx->sc->budget=0;
	sched();
	eret_to_thread(switch_context()); 
}

int sched_init(struct sched_ops *sched_ops)
{
	BUG_ON(sched_ops == NULL);

	cur_sched_ops = sched_ops;
	cur_sched_ops->sched_init();
	return 0;
}
