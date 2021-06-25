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

#include <common/smp.h>
#include <common/kprint.h>
#include <common/types.h>
#include <common/vars.h>
#include <common/mm.h>

#define NOT_BSS (0xBEEFUL)
volatile char cpu_status[PLAT_CPU_NUM] = { cpu_hang, cpu_hang, cpu_hang,
	cpu_hang
};

/* 
 * Other cores are busy looping on the addr, wake up those cores 
 * addr 就secondary的地址
 */
void enable_smp_cores(void *addr)
{
	int i = 0;
	long *secondary_boot_flag;

	/* Set current cpu status */
	cpu_status[smp_get_cpu_id()] = cpu_run;
	secondary_boot_flag = (long *)phys_to_virt(addr);
	for (i = 0; i < PLAT_CPU_NUM; i++) {
		/* Lab4 - exersice2
		 * You should set one flag to enable the APs to continue in
		 * _start of `start.S`. Then, what's the flag ? -> in init_c.c
		 * You only need to write one line of code.
		 */
		secondary_boot_flag[i] =  NOT_BSS ;

		/* Lab4 - exercise2
		 * 把 secondary_boot_flag 改为不等于0后
		 * start.S 中副CPU可以跳出循环继续运行
		 * 就会运行 secondary_init_c -> secondary_cpu_boot -> secondary_start
		 * secondary_start 会修改cpu_status
		 */

		/* Lab4 - exercise2
		 * The BSP waits for the currently initializing AP finishing
		 * before activating the next one
		 */
		while(cpu_status[i] != cpu_run){
			; //wait
		}
	}

	/* This information is printed when all CPUs finish their initialization */
	kinfo("All %d CPUs are active\n", PLAT_CPU_NUM);
}


u32 smp_get_cpu_id(void)
{
	u64 cpuid = 0;

	asm volatile ("mrs %0, tpidr_el1":"=r" (cpuid));
	return (u32) cpuid;
}
