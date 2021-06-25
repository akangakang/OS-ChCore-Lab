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

#include <common/registers.h>
#include <common/types.h>
#include <common/list.h>
#include <common/kprint.h>

#include <common/machine.h>

struct thread;

/* BUDGET represents the number of TICKs */
#define DEFAULT_BUDGET	2
#define TICK_MS		500

#define MAX_PRIO	255
#define MIN_PRIO	0
#define PRIO_NUM	(MAX_PRIO + 1)

#define NO_AFF		-1

/* Data structures */

#define	STATE_STR_LEN	20
enum thread_state {
	TS_INIT = 0,
	TS_READY,
	/* intermediate stat used by sched */
	TS_INTER,
	TS_RUNNING,
	TS_EXIT,
	/* waiting IPC or etc */
	TS_WAITING,
	/* waiting to be exited */
	TS_EXITING,
};

#define TYPE_STR_LEN	20
enum thread_type {
	TYPE_IDLE = 0,
	/* ROOT thread has all cap, it is also a user thread */
	TYPE_ROOT,
	TYPE_USER,
	TYPE_SHADOW,
	TYPE_KERNEL,
	TYPE_TESTS
};

typedef struct sched_cont {
	u32 budget;
	char pad[pad_to_cache_line(sizeof(u32))];
} sched_cont_t;

/* size in registers.h (to be used in asm) */
typedef struct arch_exec_cont {
	u64 reg[REG_NUM];
} arch_exec_cont_t;

struct thread_ctx {
	/* Executing Context */
	arch_exec_cont_t ec;

	/* Scheduling Context */
	sched_cont_t *sc;

	/* Thread Type */
	u32 type;

	/* Thread state */
	u32 state;

	/* Priority */
	u32 prio;

	/* SMP Affinity */
	s32 affinity;

	/* Current Assigned CPU */
	u32 cpuid;
};

/* Debug functions */
void print_thread(struct thread *thread);

extern char thread_type[][TYPE_STR_LEN];
extern char thread_state[][STATE_STR_LEN];

void arch_idle_ctx_init(struct thread_ctx *idle_ctx, void (*func) (void));
u64 switch_context(void);
int sched_is_runnable(struct thread *target);
int sched_is_running(struct thread *target);
int switch_to_thread(struct thread *target);

/* Global-shared kernel data */
extern struct list_head ready_queue[PLAT_CPU_NUM][PRIO_NUM];
extern struct thread *current_threads[PLAT_CPU_NUM];

/* Indirect function call may downgrade performance */
struct sched_ops {
	int (*sched_init) (void);
	int (*sched) (void);
	int (*sched_enqueue) (struct thread * thread);
	int (*sched_dequeue) (struct thread * thread);
	struct thread *(*sched_choose_thread) (void);
	void (*sched_handle_timer_irq) (void);
	/* Debug tools */
	void (*sched_top) (void);
};

/* Provided Scheduling Policies */
extern struct sched_ops rr;	/* Simple Round Robin */

/* Chosen Scheduling Policies */
extern struct sched_ops *cur_sched_ops;

int sched_init(struct sched_ops *sched_ops);

static inline int sched(void)
{
	return cur_sched_ops->sched();
}

static inline int sched_enqueue(struct thread *thread)
{
	return cur_sched_ops->sched_enqueue(thread);
}

static inline int sched_dequeue(struct thread *thread)
{
	return cur_sched_ops->sched_dequeue(thread);
}

static inline struct thread *sched_choose_thread(void)
{
	return cur_sched_ops->sched_choose_thread();
}

static inline void sched_handle_timer_irq(void)
{
	cur_sched_ops->sched_handle_timer_irq();
}
