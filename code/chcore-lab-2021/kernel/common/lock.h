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

#include <common/types.h>

struct lock {
	volatile u32 owner;		/* 现在持有锁的人 */
	char pad0[pad_to_cache_line(sizeof(u32))];

	volatile u32 next;		/* 下一个需要分发的序号 */
	char pad1[pad_to_cache_line(sizeof(u32))];
} __attribute__ ((aligned(CACHELINE_SZ)));

int lock_init(struct lock *lock);
void lock(struct lock *lock);
int try_lock(struct lock *lock);
void unlock(struct lock *lock);
int is_locked(struct lock *lock);

/* Global locks */
extern struct lock big_kernel_lock;
void kernel_lock_init(void);
void lock_kernel(void);
void unlock_kernel(void);
