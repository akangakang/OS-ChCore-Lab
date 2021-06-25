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

#include <common/kprint.h>
#include <common/lock.h>
#include <common/machine.h>
#include <common/macro.h>
#include <common/mm.h>
#include <common/smp.h>
#include <common/uart.h>
#include <common/vars.h>
#include <exception/exception.h>
#include <ipc/ipc.h>
#include <common/types.h>
#include <process/thread.h>
#include <sched/sched.h>
#include <tests/tests.h>

ALIGN(STACK_ALIGNMENT)
char kernel_stack[PLAT_CPU_NUM][KERNEL_STACK_SIZE];

int stack_backtrace();

// Test the stack backtrace function (lab 1 only)
__attribute__ ((optimize("O1")))
void stack_test(long x)
{
	kinfo("entering stack_test %d\n", x);
	if (x > 0)
		stack_test(x - 1);
	else
		stack_backtrace();
	kinfo("leaving stack_test %d\n", x);
}

void main(void *addr)
{
	/* Init uart */
	uart_init();
	kinfo("[ChCore] uart init finished\n");

	kinfo("Address of main() is 0x%lx\n", main);
	kinfo("123456 decimal is 0%o octal\n", 123456);

	stack_test(5);

	mm_init();
	kinfo("mm init finished\n");

	/* Init exception vector */
	exception_init();
	kinfo("[ChCore] interrupt init finished\n");

	/**
	 *  Lab4 - exercise 5
	 *  Initialize and then acquire the big kernel lock.
	 */
	kernel_lock_init();
	kinfo("[ChCore] lock init finished\n");

	/* 
	 * Lab4 - exercise 5
	 * 主CPU 在激活副CPU 之前需要首先获得了大内核锁
	 */
	lock_kernel();

	/* Init scheduler with specified policy. */
	sched_init(&rr);
	kinfo("[ChCore] sched init finished\n");

#ifndef TEST
	init_test();
#endif

	/* Other cores are busy looping on the addr, wake up those cores */
	enable_smp_cores(addr);
	kinfo("[ChCore] boot multicore finished\n");

#ifdef TEST
	/* Create initial thread here */
	process_create_root(TEST);
	kinfo("[ChCore] root thread init finished\n");
#else
	/* We will run the kernel test if you do not type make bin=xxx */
	run_test(true);
	break_point();
	BUG("No given TEST!");
#endif

	/** 
	 * Where the pimary CPU first returns to the user mode
	 * Leave the scheduler to do its job 
	*/
	sched();

	eret_to_thread(switch_context());

	/* Should provide panic and use here */
	BUG("[FATAL] Should never be here!\n");
}

void secondary_start(void)
{
	kinfo("AP %u is activated!\n", smp_get_cpu_id());
	exception_init_per_cpu();

	/** 
	 * Lab4 - exercise 2
	 * Inform the BSP at last to start cpu one by one
	 * Hints: use cpu_status
	*/
	cpu_status[smp_get_cpu_id()] = cpu_run;

#ifndef TEST
	run_test(false);
#endif

	/*
	 *  Lab4 - exercise 5
	 *  Acquire the big kernel lock
	 *  start()：在初始化完成之后且副CPU 返回用户态之前获取大内核锁。
	 */
	lock_kernel();
	/* Where the AP first returns to the user mode */
	sched();
	eret_to_thread(switch_context());

	/* Should provide panic and use here */
	BUG("[FATAL] Should never be here!\n");
}
