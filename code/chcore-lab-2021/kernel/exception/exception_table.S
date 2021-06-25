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
#include <common/registers.h>
#include "exception.h"
#include "esr.h"

.extern syscall_table
.extern hook_syscall
.extern lock_kernel
.extern unlock_kernel

.macro	exception_entry	label
	/* Each entry should be 0x80 aligned */
	.align	7
	b	\label
.endm

.macro handle_entry el, type
	exception_enter
	mov	x0, #\type
	mrs	x1, esr_el1
	mrs	x2, elr_el1
	bl	handle_entry_c
	exception_return
.endm

/* See more details about the bias in registers.h */
.macro	exception_enter
	sub	sp, sp, #ARCH_EXEC_CONT_SIZE
	/* 保护用户态的寄存器 */
	stp	x0, x1, [sp, #16 * 0]
	stp	x2, x3, [sp, #16 * 1]
	stp	x4, x5, [sp, #16 * 2]
	stp	x6, x7, [sp, #16 * 3]
	stp	x8, x9, [sp, #16 * 4]
	stp	x10, x11, [sp, #16 * 5]
	stp	x12, x13, [sp, #16 * 6]
	stp	x14, x15, [sp, #16 * 7]
	stp	x16, x17, [sp, #16 * 8]
	stp	x18, x19, [sp, #16 * 9]
	stp	x20, x21, [sp, #16 * 10]
	stp	x22, x23, [sp, #16 * 11]
	stp	x24, x25, [sp, #16 * 12]
	stp	x26, x27, [sp, #16 * 13]
	stp	x28, x29, [sp, #16 * 14]
	mrs	x10, sp_el0
	mrs	x11, elr_el1
	mrs	x12, spsr_el1 
	stp	x30, x10, [sp, #16 * 15]
	stp	x11, x12, [sp, #16 * 16]
.endm

.macro	exception_exit
	ldp	x11, x12, [sp, #16 * 16]
	ldp	x30, x10, [sp, #16 * 15] 
	msr	sp_el0, x10
	msr	elr_el1, x11
	msr	spsr_el1, x12
	/* 恢复用户态的寄存器 */
	ldp	x0, x1, [sp, #16 * 0]
	ldp	x2, x3, [sp, #16 * 1]
	ldp	x4, x5, [sp, #16 * 2]
	ldp	x6, x7, [sp, #16 * 3]
	ldp	x8, x9, [sp, #16 * 4]
	ldp	x10, x11, [sp, #16 * 5]
	ldp	x12, x13, [sp, #16 * 6]
	ldp	x14, x15, [sp, #16 * 7]
	ldp	x16, x17, [sp, #16 * 8]
	ldp	x18, x19, [sp, #16 * 9]
	ldp	x20, x21, [sp, #16 * 10]
	ldp	x22, x23, [sp, #16 * 11]
	ldp	x24, x25, [sp, #16 * 12]
	ldp	x26, x27, [sp, #16 * 13]
	ldp	x28, x29, [sp, #16 * 14]
	add	sp, sp, #ARCH_EXEC_CONT_SIZE
	eret
.endm


/** 	
 *  Lab4 - excercise 5
 * 	unlock the big kernel lock before returning to the user mode
 */
.macro	exception_return
	bl unlock_kernel
	exception_exit
.endm


/*
 * Vecotr table offsets from vector table base address from ARMv8 Manual
 *	Address			|	Exception Type		| 	Description
 * ============================================================================
 *	VBAR_Eln+0x000	|	 Synchronous		|	 SPSel=0
 * 		+0x080		|	  IRQ/vIRQ			|	Current EL
 *		+0x100		|	  FIQ/vFIQ			|   with Stack Pointer
 * 		+0x180		|	SError/vSError		|    shared with EL0
 * ============================================================================
 *	VBAR_Eln+0x200	|	 Synchronous		|	 SPSel=1
 * 		+0x280		|	  IRQ/vIRQ			|	Current EL
 *		+0x300		|	  FIQ/vFIQ			|   with dedicated
 * 		+0x380		|	SError/vSError		|    Stack Pointer
 * ============================================================================
 *	VBAR_Eln+0x400	|	 Synchronous		|
 * 		+0x480		|	  IRQ/vIRQ			|	Lower EL
 *		+0x500		|	  FIQ/vFIQ			|    using AArch64
 * 		+0x580		|	SError/vSError		|
 * ============================================================================
 *	VBAR_Eln+0x600	|	 Synchronous		|
 * 		+0x680		|	  IRQ/vIRQ			|     	Lower EL
 *		+0x700		|	  FIQ/vFIQ			|    using AArch32
 * 		+0x780		|	SError/vSError		|
 * ============================================================================
 */

/*
 * Lab3: Your code here
 * Use exception_entry to fill the exception vector table to redirect
 * every entry to cooresponeding entry.
 * NOTE: el1t means SP_EL0 is used as stack before exception and el1h
 * means SP_EL1 is used
 */
.align	11
EXPORT(el1_vector)
		exception_entry sync_el1t
		exception_entry irq_el1t
		exception_entry fiq_el1t
		exception_entry error_el1t

		exception_entry sync_el1h
		exception_entry irq_el1h
		exception_entry fiq_el1h
		exception_entry error_el1h

		exception_entry sync_el0_64
		exception_entry irq_el0_64
		exception_entry fiq_el0_64
		exception_entry error_el0_64

		exception_entry sync_el0_32
		exception_entry irq_el0_32
		exception_entry fiq_el0_32
		exception_entry error_el0_32
		
sync_el1t:
	handle_entry	1, SYNC_EL1t

irq_el1t:
	handle_entry 	1, IRQ_EL1t

fiq_el1t:
	handle_entry	1, FIQ_EL1t

error_el1t:
	handle_entry	1, ERROR_EL1t

sync_el1h:
	handle_entry	1, SYNC_EL1h

fiq_el1h:
	handle_entry	1, FIQ_EL1h

error_el1h:
	handle_entry	1, ERROR_EL1h

sync_el0_64:
	/* Since we cannot touch x0-x7, we need some extra work here */
	exception_enter
	mrs	x25, esr_el1
	lsr	x24, x25, #ESR_EL1_EC_SHIFT
	cmp	x24, #ESR_EL1_EC_SVC_64
	b.eq	el0_syscall
	/* Not supported exception */
	mov	x0, SYNC_EL0_64 
	mrs	x1, esr_el1
	mrs	x2, elr_el1
	bl	handle_entry_c
	exception_return

el0_syscall:
	/* 	Lab4 - excercise 5/6
	* 	Acquire the big kernel lock for syscall
	*/
	sub	sp, sp, #16 * 8
	stp	x0, x1, [sp, #16 * 0]
	stp	x2, x3, [sp, #16 * 1]
	stp	x4, x5, [sp, #16 * 2]
	stp	x6, x7, [sp, #16 * 3]
	stp	x8, x9, [sp, #16 * 4]
	stp	x10, x11, [sp, #16 * 5]
	stp	x12, x13, [sp, #16 * 6]
	stp	x14, x15, [sp, #16 * 7]

	bl lock_kernel

	ldp	x0, x1, [sp, #16 * 0]
	ldp	x2, x3, [sp, #16 * 1]
	ldp	x4, x5, [sp, #16 * 2]
	ldp	x6, x7, [sp, #16 * 3]
	ldp	x8, x9, [sp, #16 * 4]
	ldp	x10, x11, [sp, #16 * 5]
	ldp	x12, x13, [sp, #16 * 6]
	ldp	x14, x15, [sp, #16 * 7]
	add	sp, sp, #16 * 8

	adr	x27, syscall_table		// syscall table in x27
	uxtw	x16, w8				// syscall number in x16
	ldr	x16, [x27, x16, lsl #3]		// find the syscall entry
	blr	x16

	/* Ret from syscall */
	//bl	disable_irq
	str	x0, [sp] /* set the return value of the syscall */
	exception_return
	
irq_el1h:
	exception_enter
	mov	x0, IRQ_EL1h
	bl	handle_irq
	/* should nerver reach here */
	b .

irq_el0_64:
	exception_enter
	mov	x0, IRQ_EL0_64
	bl	handle_irq
	/* should nerver reach here */
	b .

fiq_el0_64:
	handle_entry	0, FIQ_EL0_64

error_el0_64:
	handle_entry	0, ERROR_EL0_64

sync_el0_32:
	handle_entry	0, SYNC_EL0_32

irq_el0_32:
	handle_entry	0, IRQ_EL0_32

fiq_el0_32:
	handle_entry	0, FIQ_EL0_32

error_el0_32:
	handle_entry	0, ERROR_EL0_32

/* void eret_to_thread(u64 sp) */
BEGIN_FUNC(eret_to_thread)
	mov	sp, x0
	exception_return
END_FUNC(eret_to_thread)
