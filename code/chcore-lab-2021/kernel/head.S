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

#include <common/asm.h>
#include <common/vars.h>

BEGIN_FUNC(start_kernel)
    /* 
     * Code in bootloader specified only the primary 
     * cpu with MPIDR = 0 can be boot here. So we directly
     * set the TPIDR_EL1 to 0, which represent the logical
     * cpuid in the kernel 
     */
    mov     x3, #0
    msr     TPIDR_EL1, x3

    ldr     x2, =kernel_stack
    add     x2, x2, KERNEL_STACK_SIZE
    mov     sp, x2
    bl      main
END_FUNC(start_kernel)

BEGIN_FUNC(secondary_cpu_boot)
    /* We store the logical cpuid in TPIDR_EL1 */
    msr     TPIDR_EL1, x0               /* write x0(cpu_id) into TPIDR_EL1 */

    mov     x1, #KERNEL_STACK_SIZE
    mul     x2, x0, x1
    ldr     x3, =kernel_stack
    add     x2, x2, x3
    add     x2, x2, KERNEL_STACK_SIZE
    mov     sp, x2
    bl      secondary_start
END_FUNC(secondary_cpu_boot)