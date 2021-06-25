#pragma once

#include <common/lock.h>
#include <tests/barrier.h>

extern struct lock test_lock;

void init_test(void);
void run_test(bool);

/**
 * Locking
 */
void tst_mutex(bool);
void tst_big_lock(bool);

/**
 * Scheduler
 */
void tst_sched_cooperative(bool);
void tst_sched_preemptive(bool);
void tst_sched_affinity(bool);
void tst_sched(bool);
