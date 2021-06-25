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
#include <exception/irq.h>
#include <sched/context.h>

/* in arch/sched/idle.S */
void idle_thread_routine(void);

/*
 * rr_ready_queue
 * Per-CPU ready queue for ready tasks.
 */
struct list_head rr_ready_queue[PLAT_CPU_NUM];

/*
 * RR policy also has idle threads.
 * When no active user threads in ready queue,
 * we will choose the idle thread to execute.
 * Idle thread will **NOT** be in the RQ.
 */
struct thread idle_threads[PLAT_CPU_NUM];

/*
 * Lab4 - exercise 7
 * Sched_enqueue
 * Put `thread` at the end of ready queue of assigned `affinity`.
 * If affinity = NO_AFF, assign the core to the current cpu.
 * If the thread is IDEL thread, do nothing!
 * Do not forget to check if the affinity is valid!
 */
int rr_sched_enqueue(struct thread *thread)
{
	if (thread == NULL || thread->thread_ctx == NULL || thread->thread_ctx->state == TS_READY)
	{
		return -EINVAL;
	}
	/* If the thread is IDEL thread, do nothing! */
	if (thread->thread_ctx->type == TYPE_IDLE)
	{
		return 0;
	}

	/* 
	 * If affinity = NO_AFF, assign the core to the current cpu.
	 * 将线程给他指定的cpu运行
	 */
	u32 cpu_id = smp_get_cpu_id();
	if (thread->thread_ctx->affinity != NO_AFF)
	{
		cpu_id = thread->thread_ctx->affinity;
		if (cpu_id >= PLAT_CPU_NUM)
		{
			return -EINVAL;
		}
	}

	list_append(&thread->ready_queue_node, &rr_ready_queue[cpu_id]);
	thread->thread_ctx->state = TS_READY;
	thread->thread_ctx->cpuid = cpu_id; /* [ERROR]: tst_sched_param:120 threads[i]->thread_ctx->cpuid != cpuid*/
	return 0;
}

/*
 * Lab4 - exercise 7
 * Sched_dequeue
 * remove `thread` from its current residual ready queue
 * Do not forget to add some basic parameter checking
 */
int rr_sched_dequeue(struct thread *thread)
{
	if (thread == NULL || thread->thread_ctx == NULL || thread->thread_ctx->state != TS_READY)
	{
		return -EINVAL;
	}

	/* 如果是空闲线程 则不用出队 */
	if (thread->thread_ctx->type == TYPE_IDLE)
	{
		// thread->thread_ctx->state = TS_INTER;
		return 0;
	}

	list_del(&thread->ready_queue_node);
	thread->thread_ctx->state = TS_INTER;
	return 0;
}

/*
 * Lab4 - exercise 7
 * The helper function
 * Choose an appropriate thread and dequeue from ready queue
 * 
 * If there is no ready thread in the current CPU's ready queue, 
 * choose the idle thread of the CPU.
 * 
 * Do not forget to check the type and 
 * state of the chosen thread
 */
struct thread *rr_sched_choose_thread(void)
{
	/* 首先检查CPU 核心的rr_ready_queue是否为空
	 * 如果是，rr_choose_thread返回CPU 核心自己的空闲线程 
	 */
	u32 cpu_id = smp_get_cpu_id();
	if (list_empty(&(rr_ready_queue[cpu_id])))
	{
		return &(idle_threads[cpu_id]);
	}

	/*
	 * 如果没有，它将选择rr_ready_queue的队首
	 * 并调用rr_sched_dequeue()使该队首出队，然后返回该队首
	 */
	struct thread *chosen_thread = list_entry(rr_ready_queue[cpu_id].next, struct thread, ready_queue_node);
	if (rr_sched_dequeue(chosen_thread) < 0)
	{
		return NULL;
	}
	return chosen_thread;
}

static inline void rr_sched_refill_budget(struct thread *target, u32 budget)
{
	target->thread_ctx->sc->budget = budget;
}

/*
 * Lab4 - exercise 7
 * Schedule a thread to execute.
 * This function will suspend current running thread, if any, and schedule
 * another thread from `rr_ready_queue[cpu_id]`.
 * 
 * Hints:
 * Macro DEFAULT_BUDGET defines the value for resetting thread's budget.
 * After you get one thread from rr_sched_choose_thread, pass it to
 * switch_to_thread() to prepare for switch_context().
 * Then ChCore can call eret_to_thread() to return to user mode.
 */
int rr_sched(void)
{
	/*  
	 * 调度器应只能在某个线程预算等于零时才能调度该线程
	 */
	if (current_thread != NULL && current_thread->thread_ctx != NULL && current_thread->thread_ctx->sc != NULL && current_thread->thread_ctx->sc->budget != 0)
	{
		return 0;
	}
	/*
	 * 首先检查当前是否正在运行某个线程
	 * 如果是，它将调用rr_sched_enqueue()将线程添加回rr_ready_queue
	 * rr_ready_queue 里面是不运行的线程哦
	 */
	if (current_thread != NULL)
	{
		rr_sched_enqueue(current_thread);
	}

	struct thread *target_thread;
	if ((target_thread = rr_sched_choose_thread()) == NULL)
	{
		return -EINVAL;
	}

	/* resetting thread's budget */
	rr_sched_refill_budget(target_thread, DEFAULT_BUDGET);

	return switch_to_thread(target_thread);
}

/*
 * Initialize the per thread queue and idle thread.
 */
int rr_sched_init(void)
{
	int i = 0;

	/* Initialize global variables */
	for (i = 0; i < PLAT_CPU_NUM; i++)
	{
		current_threads[i] = NULL;
		init_list_head(&rr_ready_queue[i]);
	}

	/* Initialize one idle thread for each core and insert into the RQ */
	for (i = 0; i < PLAT_CPU_NUM; i++)
	{
		/* Set the thread context of the idle threads */
		BUG_ON(!(idle_threads[i].thread_ctx = create_thread_ctx()));
		/* We will set the stack and func ptr in arch_idle_ctx_init */
		init_thread_ctx(&idle_threads[i], 0, 0, MIN_PRIO, TYPE_IDLE, i);
		/* Call arch-dependent function to fill the context of the idle
		 * threads */
		arch_idle_ctx_init(idle_threads[i].thread_ctx,
						   idle_thread_routine);
		/* Idle thread is kernel thread which do not have vmspace */
		idle_threads[i].vmspace = NULL;
	}
	kdebug("Scheduler initialized. Create %d idle threads.\n", i);

	return 0;
}

/*
 * Lab4 - exercise 11
 * Handler called each time a timer interrupt is handled
 * Do not forget to call sched_handle_timer_irq() in proper code location.
 */
void rr_sched_handle_timer_irq(void)
{
	if (current_thread != NULL && current_thread->thread_ctx->sc->budget > 0)
	{
		current_thread->thread_ctx->sc->budget--;
	}
}

struct sched_ops rr = {
	.sched_init = rr_sched_init,
	.sched = rr_sched,
	.sched_enqueue = rr_sched_enqueue,
	.sched_dequeue = rr_sched_dequeue,
	.sched_choose_thread = rr_sched_choose_thread,
	.sched_handle_timer_irq = rr_sched_handle_timer_irq,
};
