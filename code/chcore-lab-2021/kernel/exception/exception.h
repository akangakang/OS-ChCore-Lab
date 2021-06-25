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

#define SYNC_EL1t		0
#define IRQ_EL1t		1
#define FIQ_EL1t		2
#define ERROR_EL1t		3

#define SYNC_EL1h		4
#define IRQ_EL1h		5
#define FIQ_EL1h		6
#define ERROR_EL1h		7

#define SYNC_EL0_64             8
#define IRQ_EL0_64              9
#define FIQ_EL0_64              10
#define ERROR_EL0_64            11

#define SYNC_EL0_32             12
#define IRQ_EL0_32              13
#define FIQ_EL0_32              14
#define ERROR_EL0_32            15

#ifndef __ASM__
#include <common/types.h>

/* assembly helper functions */
void set_exception_vector(void);
void exception_init(void);
void exception_init_per_cpu(void);

void eret_to_thread(u64 sp);
#endif				/* __ASM__ */
