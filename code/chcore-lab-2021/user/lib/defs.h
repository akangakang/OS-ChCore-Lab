#pragma once

/* virtual memory rights */
#define VM_READ  (1 << 0)
#define VM_WRITE (1 << 1)
#define VM_EXEC  (1 << 2)

/* PMO types */
#define PMO_ANONYM 0
#define PMO_DATA   1

/* a thread's own process */
#define SELF_CAP   0

#define MAX_PRIO   255

/* magic numbers */
#define CHILD_THREAD_STACK_BASE         (0x10000000)
#define CHILD_THREAD_STACK_SIZE         (0x10000)
#define CHILD_THREAD_PRIO               (MAX_PRIO - 1)

#define MAIN_THREAD_STACK_BASE		(0x8000000)
#define MAIN_THREAD_STACK_SIZE		(0x10000)
#define MAIN_THREAD_PRIO		(MAX_PRIO - 1)

#define ROUND_UP(x, n)		(((x) + (n) - 1) & ~((n) - 1))
#define ROUND_DOWN(x, n)	((x) & ~((n) - 1))

#define PAGE_SIZE   0x1000
