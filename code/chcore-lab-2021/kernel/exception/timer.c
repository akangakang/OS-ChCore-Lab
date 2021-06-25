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

#include <common/kprint.h>
#include <common/machine.h>
#include <common/smp.h>
#include <common/tools.h>
#include <common/types.h>
#include <exception/irq.h>
#include <exception/timer.h>
#include <process/thread.h>
#include <sched/sched.h>

u64 cntv_tval;

/* Per core IRQ SOURCE MMIO address */
u64 core_timer_irqcntl[PLAT_CPU_NUM] = {
	CORE0_TIMER_IRQCNTL, CORE1_TIMER_IRQCNTL, CORE2_TIMER_IRQCNTL,
	CORE3_TIMER_IRQCNTL
};

void timer_init(void)
{
	u64 cur_freq = 0;
	u64 cur_cnt = 0;
	u64 count_down = 0;
	u64 timer_ctl = 0;
	u32 cpuid = smp_get_cpu_id();

	/* Since QEMU only emulate the generic timer, we use the generic timer
	 * here */
	asm volatile ("mrs %0, cntpct_el0":"=r" (cur_cnt));
	kdebug("timer init cntpct_el0 = %lu\n", cur_cnt);
	asm volatile ("mrs %0, cntfrq_el0":"=r" (cur_freq));
	kdebug("timer init cntfrq_el0 = %lu\n", cur_freq);

	/* Calculate the tv */
	cntv_tval = (cur_freq * TICK_MS / 1000);
	kdebug("CPU freq %lu, set timer %lu\n", cur_freq, cntv_tval);

	/* set the timervalue here */
	asm volatile ("msr cntv_tval_el0, %0"::"r" (cntv_tval));
	asm volatile ("mrs %0, cntv_tval_el0":"=r" (count_down));
	kdebug("timer init cntv_tval_el0 = %lu\n", count_down);

	put32(core_timer_irqcntl[cpuid], INT_SRC_TIMER3);

	/* Set the control register */
	timer_ctl = 0 << 1 | 1;	/* IMASK = 0 ENABLE = 1 */
	asm volatile ("msr cntv_ctl_el0, %0"::"r" (timer_ctl));
	asm volatile ("mrs %0, cntv_ctl_el0":"=r" (timer_ctl));
	kdebug("timer init cntv_ctl_el0 = %lu\n", timer_ctl);
	/* enable interrupt controller */
	return;
}

void plat_handle_timer_irq(void)
{
	asm volatile ("msr cntv_tval_el0, %0"::"r" (cntv_tval));
}

void plat_disable_timer(void)
{
	u64 timer_ctl = 0x0;

	asm volatile ("msr cntv_ctl_el0, %0"::"r" (timer_ctl));
}

void plat_enable_timer(void)
{
	u64 timer_ctl = 0x1;

	asm volatile ("msr cntv_tval_el0, %0"::"r" (cntv_tval));
	asm volatile ("msr cntv_ctl_el0, %0"::"r" (timer_ctl));
}

void handle_timer_irq(void)
{
	plat_handle_timer_irq();
	sched_handle_timer_irq();
}
