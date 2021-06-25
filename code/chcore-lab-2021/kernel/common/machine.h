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

#pragma once

#include <common/vars.h>

/* raspi3 config */
#define PLAT_CPU_NUM    4

// Timers interrupt control registers
#define IMER_IRQCNTL_BASE	(KBASE + 0x40000040)
#define CORE0_TIMER_IRQCNTL	(IMER_IRQCNTL_BASE + 0x0)
#define CORE1_TIMER_IRQCNTL	(IMER_IRQCNTL_BASE + 0x4)
#define CORE2_TIMER_IRQCNTL	(IMER_IRQCNTL_BASE + 0x8)
#define CORE3_TIMER_IRQCNTL	(IMER_IRQCNTL_BASE + 0xc)
#define INT_SRC_TIMER3		0x008

// IRQ & FIQ source registers
#define IRQ_BASE	(KBASE + 0x40000060)
#define CORE0_IRQ	(IRQ_BASE + 0x0)
#define CORE1_IRQ	(IRQ_BASE + 0x4)
#define CORE2_IRQ	(IRQ_BASE + 0x8)
#define CORE3_IRQ	(IRQ_BASE + 0xc)
