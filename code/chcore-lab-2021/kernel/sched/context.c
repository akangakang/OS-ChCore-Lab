/*
 * Copyright (c) 2020 Institute of Parallel And Distributed Systems (IPADS),
 * Shanghai Jiao Tong University (SJTU) OS-Lab-2020 (i.e., ChCore) is licensed
 * under the Mulan PSL v1. You can use this software according to the terms and
 * conditions of the Mulan PSL v1. You may obtain a copy of Mulan PSL v1 at:
 *   http://license.coscl.org.cn/MulanPSL
 *   THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY
 * KIND, EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
 * NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE. See the
 * Mulan PSL v1 for more details.
 */

#include <common/kmalloc.h>
#include <common/kprint.h>
#include <common/smp.h>
#include <common/util.h>
#include <common/registers.h>
#include <process/thread.h>
#include <sched/sched.h>

struct thread_ctx *create_thread_ctx(void)
{
	void *kernel_stack;

	kernel_stack = kzalloc(DEFAULT_KERNEL_STACK_SZ);
	if (kernel_stack == NULL) {
		kwarn("create_thread_ctx fails due to lack of memory\n");
		return NULL;
	}
	return kernel_stack + DEFAULT_KERNEL_STACK_SZ -
	    sizeof(struct thread_ctx);
}

void destroy_thread_ctx(struct thread *thread)
{
	void *kernel_stack;
	BUG_ON(!thread->thread_ctx);
	kernel_stack = (void *)thread->thread_ctx - DEFAULT_KERNEL_STACK_SZ +
	    sizeof(struct thread_ctx);
	kfree(kernel_stack);
}

void init_thread_ctx(struct thread *thread, u64 stack, u64 func, u32 prio,
		     u32 type, s32 aff)
{
	/*
	 * Lab3: Your code here
	 * You need to initialize a thread's context here for later eret_to_thread(context) to work properly
	 *
	 * In this function, you need setup the registers and type of the thread's context, including:
	 * 1. Set SP_EL0 to stack.
	 * 2. Set ELR_EL1 to the entrypoint of this thread.
	 * 3. Set SPSR_EL1 to SPSR_EL1_EL0t as the properly PSTATE. One of the most
	 *      important field is SPSR_EL1[3:0] (set as 0 to indicate eret to EL0 in eret_to_thread)
	 *
	 * Check out macro in registers.h for more help
	 */

	/* Fill the context of the thread */
	// arch_set_thread_stack(thread,stack);					// Set SP_EL0 to stack.
	thread->thread_ctx->ec.reg[SP_EL0] = stack;
	// arch_set_thread_next_ip(thread,func);					// Set ELR_EL1 to the entrypoint of this thread.
	thread->thread_ctx->ec.reg[ELR_EL1] = func;
	thread->thread_ctx->ec.reg[SPSR_EL1] = SPSR_EL1_EL0t;	// Set SPSR_EL1 to SPSR_EL1_EL0t

	/* Set thread type */

	thread->thread_ctx->type = type;

	/* Set the priority and state of the thread */
	thread->thread_ctx->prio = prio;
	thread->thread_ctx->state = TS_INIT;

	/* Set the cpuid and affinity */
	thread->thread_ctx->affinity = aff;

	/* Set the budget of the thread */
	thread->thread_ctx->sc = kmalloc(sizeof(sched_cont_t));
	thread->thread_ctx->sc->budget = DEFAULT_BUDGET;
}

u64 arch_get_thread_stack(struct thread *thread)
{
	return thread->thread_ctx->ec.reg[SP_EL0];
}

void arch_set_thread_stack(struct thread *thread, u64 stack)
{
	thread->thread_ctx->ec.reg[SP_EL0] = stack;
}

void arch_set_thread_return(struct thread *thread, u64 ret)
{
	thread->thread_ctx->ec.reg[X0] = ret;
}

void arch_set_thread_next_ip(struct thread *thread, u64 ip)
{
	/* Currently, we use fault PC to store the next ip */
	// thread->thread_ctx->ec.reg[FaultPC] = ip;
	/* Only required when we need to change PC */
	/* Maybe update ELR_EL1 directly */
	thread->thread_ctx->ec.reg[ELR_EL1] = ip;
}

u64 arch_get_thread_next_ip(struct thread *thread)
{
	return thread->thread_ctx->ec.reg[ELR_EL1];
}

void arch_set_thread_info_page(struct thread *thread, u64 info_page_addr)
{
	thread->thread_ctx->ec.reg[X0] = info_page_addr;
}

void arch_set_thread_arg(struct thread *thread, u64 arg)
{
	thread->thread_ctx->ec.reg[X0] = arg;
}

void arch_enable_interrupt(struct thread *thread)
{
	thread->thread_ctx->ec.reg[SPSR_EL1] &= ~SPSR_EL1_IRQ;
}

void arch_disable_interrupt(struct thread *thread)
{
	thread->thread_ctx->ec.reg[SPSR_EL1] |= SPSR_EL1_IRQ;
}
