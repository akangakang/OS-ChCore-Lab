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

#include "exception.h"
#include "esr.h"
#include "timer.h"
#include <common/kprint.h>
#include <common/lock.h>
#include <common/smp.h>
#include <common/types.h>
#include <common/util.h>
#include <exception/irq.h>
#include <exception/pgfault.h>
#include <sched/sched.h>

u8 irq_handle_type[MAX_IRQ_NUM];

void exception_init_per_cpu(void)
{
	/**
	 * Lab4- exercise 10
	 *
	 * Uncomment the timer_init() when you are handling preemptive
	 * shceduling
	 */
	timer_init();

	/**
	 * Lab3: Your code here
	 * Lab4- exercise 10 :注释中断
	 * Setup the exception vector with the asm function written in exception.S
	 */

	/*
	 * 本实验在内核态下将禁用中断。这一功能是在硬件的帮助下实现的。
	 * 当异常触发时，中断即被禁用。当汇编代码eret被调用时，中断则会被重新启用
	 */
	disable_irq();
	set_exception_vector(); // 将向量表的首地址放入vbar_el1寄存器中
	// enable_irq();
}

void exception_init(void)
{
	exception_init_per_cpu();
	memset(irq_handle_type, HANDLE_KERNEL, MAX_IRQ_NUM);
}

void handle_entry_c(int type, u64 esr, u64 address)
{
	/** 
	 * Lab4 - exercise 5
	 * Acquire the big kernel lock, if the exception is not from kernel
	 * 在该异常处理函数的第一行获取大内核锁。
	 * 因为在内核态下也可能会发生异常，所以如果异常是在内核中捕获的，则不应获取大内核锁。
	 */
	/*
	 * 是不是内核态 看type
	 */
	if(type >= SYNC_EL0_64){ 
		lock_kernel();
	}

	/* ec: exception class */
	u32 esr_ec = GET_ESR_EL1_EC(esr);

	kdebug("Interrupt type: %d, ESR: 0x%lx, Fault address: 0x%lx, EC 0b%lx\n",
		   type, esr, address, esr_ec);
	/* Dispatch exception according to EC */
	switch (esr_ec)
	{
		/*
		 * Lab3: Your code here
		 * Handle exceptions as required in the lab document. Checking exception codes in
		 * esr.h may help.
		 */
	case ESR_EL1_EC_UNKNOWN:
		kinfo(UNKNOWN);
		sys_exit(-ESUPPORT);
		break;

	case ESR_EL1_EC_IABT_LEL:
	case ESR_EL1_EC_IABT_CEL:
		sys_exit(0);
		break;

	case ESR_EL1_EC_DABT_LEL:
	case ESR_EL1_EC_DABT_CEL:
		do_page_fault(esr, address);
		break;

	default:
		kdebug("Unsupported Exception ESR %lx\n", esr);
		break;
	}
}
