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

#include <common/macro.h>

/* ARMv8 AA64 REGISTERS */

/* SPSR_EL1 Register aarch64 (FROM ARM-ARM C5-395) */
/* Holds the saved process state when an exception is taken to EL1 */
#define SPSR_EL1_N                  BIT(31)	/* N condition flag */
#define SPSR_EL1_Z                  BIT(30)	/* Z condition flag */
#define SPSR_EL1_C                  BIT(29)	/* C condition flag */
#define SPSR_EL1_V                  BIT(28)	/* V condition flag */
#define SPSR_EL1_DIT                BIT(24)	/* Data Independent Timing PSTATE.DIT ARMv8.4 */
#define SPSR_EL1_UAO                BIT(23)	/* User Access Override PSTATE.UAO ARMv8.2 */
#define SPSR_EL1_PAN                BIT(22)	/* Privileged Access Never PSTATE.PAN ARMv8.1 */
#define SPSR_EL1_SS                 BIT(21)	/* Software Step PSTATE.SS */
#define SPSR_EL1_IL                 BIT(20)	/* Illegal Execution state PSTATE.IL */
#define SPSR_EL1_DEBUG              BIT(9)	/* Debug mask */
#define SPSR_EL1_SERROR             BIT(8)	/* SERROR mask */
#define SPSR_EL1_IRQ                BIT(7)	/* IRQ mask */
#define SPSR_EL1_FIQ                BIT(6)	/* FIQ mask */
#define SPSR_EL1_M                  BIT(4)	/* Exception taken from AArch64 */
#define SPSR_EL1_EL0t               0b0000
#define SPSR_EL1_EL1t               0b0100
#define SPSR_EL1_EL1h               0b0101

/* SPSR_EL1 DEFAULT */
#define SPSR_EL1_KERNEL             SPSR_EL1_EL1h
#define SPSR_EL1_USER               SPSR_EL1_EL0t

/* SCTLR_EL1 System Control Register aarch64 (FROM ARM-ARM D12-3081) */

#define SCTLR_EL1_EnIA              BIT(31)	/* Controls enabling of pointer authentication */
#define SCTLR_EL1_EnIB              BIT(30)	/* Controls enabling of pointer authentication */
#define SCTLR_EL1_LSMAOE            BIT(29)	/* Load Multiple and Store Multiple Atomicity and Ordering Enable */
#define SCTLR_EL1_nTLSMD            BIT(28)	/* No Trap Load Multiple and Store Multiple to Device-nGRE/Device-nGnRE/Device-nGnRnE memory */
#define SCTLR_EL1_EnDA              BIT(27)	/* Controls enabling of pointer authentication */
#define SCTLR_EL1_UCI               BIT(26)	/* Traps EL0 execution of cache maintenance instructions to EL1, from AArch64 state only */
#define SCTLR_EL1_EE                BIT(25)	/* Endianness of data accesses at EL1, and stage 1 translation table walks in the EL1&0 translation regime */
#define SCTLR_EL1_E0E               BIT(24)	/* Endianness of data accesses at EL0 */
#define SCTLR_EL1_SPAN              BIT(23)	/* Set Privileged Access Never, on taking an exception to EL1 */
#define SCTLR_EL1_IESB              BIT(21)	/* Implicit error synchronization event enable */
#define SCTLR_EL1_WXN               BIT(19)	/* Write permission implies XN (Execute-never) */
#define SCTLR_EL1_nTWE              BIT(18)	/* Traps EL0 execution of WFE instructions to EL1, from both Execution states */
#define SCTLR_EL1_nTWI              BIT(16)	/* Traps EL0 execution of WFI instructions to EL1, from both Execution states */
#define SCTLR_EL1_UCT               BIT(15)	/* Traps EL0 accesses to the CTR_EL0 to EL1, from AArch64 state only */
#define SCTLR_EL1_DZE               BIT(14)	/* Traps EL0 execution of DC ZVA instructions to EL1, from AArch64 state only */
#define SCTLR_EL1_EnDB              BIT(13)	/* Controls enabling of pointer authentication */
#define SCTLR_EL1_I                 BIT(12)	/* Instruction access Cacheability control, for accesses at EL0 and EL1 */
#define SCTLR_EL1_UMA               BIT(9)	/* User Mask Access: Traps EL0 execution of MSR and MRS instructions that access the PSTATE.{D, A, I, F} */
#define SCTLR_EL1_SED               BIT(8)	/* SETEND instruction disable. Disables SETEND instructions at EL0 using AArch32 */
#define SCTLR_EL1_ITD               BIT(7)	/* IT Disable. Disables some uses of IT instructions at EL0 using AArch32 */
#define SCTLR_EL1_nAA               BIT(6)	/* Non-aligned access. This bit controls generation of Alignment faults at EL1 and EL0 under certain conditions */
#define SCTLR_EL1_CP15BEN           BIT(5)	/* System instruction memory barrier enable (AArch32) */
#define SCTLR_EL1_SA0               BIT(4)	/* SP Alignment check enable for EL0 */
#define SCTLR_EL1_SA                BIT(3)	/* SP Alignment check */
#define SCTLR_EL1_C                 BIT(2)	/* Cacheability control for data accesses */
#define SCTLR_EL1_A                 BIT(1)	/* Alignment check enable */
#define SCTLR_EL1_M                 BIT(0)	/* MMU enable for EL1 and EL0 stage 1 address translation */

#ifndef __ASM__
/* Types of the registers */
enum reg_type {
	X0 = 0,			/* 0x00 */
	X1 = 1,			/* 0x08 */
	X2 = 2,			/* 0x10 */
	X3 = 3,			/* 0x18 */
	X4 = 4,			/* 0x20 */
	X5 = 5,			/* 0x28 */
	X6 = 6,			/* 0x30 */
	X7 = 7,			/* 0x38 */
	X8 = 8,			/* 0x40 */
	X9 = 9,			/* 0x48 */
	X10 = 10,		/* 0x50 */
	X11 = 11,		/* 0x58 */
	X12 = 12,		/* 0x60 */
	X13 = 13,		/* 0x68 */
	X14 = 14,		/* 0x70 */
	X15 = 15,		/* 0x78 */
	X16 = 16,		/* 0x80 */
	X17 = 17,		/* 0x88 */
	X18 = 18,		/* 0x90 */
	X19 = 19,		/* 0x98 */
	X20 = 20,		/* 0xa0 */
	X21 = 21,		/* 0xa8 */
	X22 = 22,		/* 0xb0 */
	X23 = 23,		/* 0xb8 */
	X24 = 24,		/* 0xc0 */
	X25 = 25,		/* 0xc8 */
	X26 = 26,		/* 0xd0 */
	X27 = 27,		/* 0xd8 */
	X28 = 28,		/* 0xe0 */
	X29 = 29,		/* 0xe8 */
	X30 = 30,		/* 0xf0 */
	SP_EL0 = 31,		/* 0xf8 */
	ELR_EL1 = 32,		/* 0x100 NEXT PC */
	SPSR_EL1 = 33,		/* 0x108 */
	TPIDR_EL0 = 34
};
#endif				/* ASMCODE */

#define REG_NUM                 35

#define SZ_U64                  8
#define ARCH_EXEC_CONT_SIZE 	REG_NUM*SZ_U64	/* Cannot use sizeof in asm */
