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
#include <common/sync.h>
#include <common/errno.h>
#include <common/kprint.h>
#include <common/lock.h>
#include <common/macro.h>
#include <common/types.h>

struct lock big_kernel_lock;

int lock_init(struct lock *lock)
{
	BUG_ON(!lock);
	/* Initialize ticket lock */
	lock->owner = 0;
	lock->next = 0;
	return 0;
}

/**
 * Lock the ticket lock
 * This function will block until the lock is held
*/
void lock(struct lock *lock)
{
	u32 lockval = 0, newval = 0, ret = 0;

	BUG_ON(!lock);

	/** 
	 * The following asm code means:
	 * 
	 * lock->next = fetch_and_add(1);
	 * while(lock->next != lock->owner);
	 */
	asm volatile("       prfm    pstl1strm, %3\n"
				 "1:     ldaxr   %w0, %3\n"
				 "       add     %w1, %w0, #0x1\n"
				 "       stxr    %w2, %w1, %3\n"
				 "       cbnz    %w2, 1b\n"
				 "2:     ldar    %w2, %4\n"
				 "       cmp     %w0, %w2\n"
				 "       b.ne    2b\n"
				 : "=&r"(lockval), "=&r"(newval),
				   "=&r"(ret), "+Q"(lock->next)
				 : "Q"(lock->owner)
				 : "memory");
}

/**
 * Try to lock the ticket lock
 * Return 0 if succeed, -1 otherwise
*/
int try_lock(struct lock *lock)
{

	u32 lockval = 0, newval = 0, ret = 0, ownerval = 0;

	BUG_ON(!lock);
	asm volatile("       prfm    pstl1strm, %4\n"
				 "       ldaxr   %w0, %4\n"
				 "       ldar    %w3, %5\n"
				 "       add     %w1, %w0, #0x1\n"
				 "       cmp     %w0, %w3\n"
				 "       b.ne    1f\n"
				 "       stxr    %w2, %w1, %4\n"
				 "       cbz     %w2, 2f\n"
				 "1:     mov     %w2, #0xffffffffffffffff\n" /* fail */
				 "       b       3f\n"
				 "2:     mov     %w2, #0x0\n" /* success */
				 "       dmb     ish\n"		  /* barrier */
				 "3:\n"
				 : "=&r"(lockval), "=&r"(newval), "=&r"(ret),
				   "=&r"(ownerval), "+Q"(lock->next)
				 : "Q"(lock->owner)
				 : "memory");
	return ret;
}

/**
 * Unlock the ticket lock
*/
void unlock(struct lock *lock)
{
	BUG_ON(!lock);
	asm volatile("dmb ish");

	/** 
	 * Lab4 - exercise 4
	 * Unlock the ticket lock here
	 * Your code should be no more than 5 lines
	*/
	lock->owner++;
}

/** 
 * Lab4 - exercise 4
 * Check whether the ticket lock is locked
 * Return 1 if locked, 0 otherwise
 * Your code should be no more than 5 lines
*/
int is_locked(struct lock *lock)
{
	return (lock->owner < lock->next);
}

/**
 * 	Lab4 - exercise 5
 * 	Initialization of the big kernel lock
 */
void kernel_lock_init(void)
{
	lock_init(&big_kernel_lock);
}

/**
 * 	Lab4 - exercise 5
 * 	Acquire the big kernel lock
 */
void lock_kernel(void)
{
	lock(&big_kernel_lock);
}

/**
 * 	Lab4 - exercise 5
 * 	Release the big kernel lock
 */
void unlock_kernel(void)
{
	unlock(&big_kernel_lock);
}
