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

#include <common/util.h>

#include "thread_env.h"

/*
 * Setup the initial environment for a user process (main thread).
 *
 * According to Libc convention, we current set the environment
 * on the user stack.
 *
 */

#define AT_NULL   0		/* end of vector */
#define AT_IGNORE 1		/* entry should be ignored */
#define AT_EXECFD 2		/* file descriptor of program */
#define AT_PHDR   3		/* program headers for program */
#define AT_PHENT  4		/* size of program header entry */
#define AT_PHNUM  5		/* number of program headers */
#define AT_PAGESZ 6		/* system page size */
#define AT_BASE   7		/* base address of interpreter */
#define AT_FLAGS  8		/* flags */
#define AT_ENTRY  9		/* entry point of program */
#define AT_NOTELF 10		/* program is not ELF */
#define AT_UID    11		/* real uid */
#define AT_EUID   12		/* effective uid */
#define AT_GID    13		/* real gid */
#define AT_EGID   14		/* effective gid */
#define AT_PLATFORM 15		/* string identifying CPU for optimizations */
#define AT_HWCAP  16		/* arch dependent hints at CPU capabilities */
#define AT_CLKTCK 17		/* frequency at which times() increments */
/* AT_* values 18 through 22 are reserved */
#define AT_SECURE 23		/* secure mode boolean */
/* string identifying real platform, may differ from AT_PLATFORM. */
#define AT_BASE_PLATFORM 24
#define AT_RANDOM 25		/* address of 16 random bytes */
#define AT_HWCAP2 26		/* extension of AT_HWCAP */
#define AT_EXECFN 31		/* filename of program */

const char PLAT[] = "aarch64";

/*
 * For setting up the stack (env) of some process.
 *
 * env: stack top address used by kernel
 * top_vaddr: stack top address mapped to user
 */
void prepare_env(char *env, u64 top_vaddr, struct process_metadata *meta,
		 char *name)
{
	int i;
	char *name_str;
	char *plat_str;
	u64 *buf;

	env -= ENV_SIZE_ON_STACK;

	/* clear env */
	memset(env, 0, ENV_SIZE_ON_STACK);

	/* strings */
	/* the last 64 bytes */
	name_str = env + 0x1000 - 64;
	i = 0;
	while (name[i] != '\0') {
		name_str[i] = name[i];
		++i;
	}

	/* the second last 64 bytes */
	plat_str = env + 0x1000 - 2 * 64;
	i = 0;
	while (PLAT[i] != '\0') {
		plat_str[i] = PLAT[i];
		++i;
	}

	buf = (u64 *) env;
	/* argc */
	*buf = 1;

	/* argv */
	*(buf + 1) = top_vaddr - 64;
	*(buf + 2) = (u64) NULL;
	/* add more argv here */

	/* envp */
	*(buf + 3) = (u64) NULL;
	/* add more envp here */

	/* auxv */
	i = 4;
	*(buf + i + 0) = AT_SECURE;
	*(buf + i + 1) = 0;

	*(buf + i + 2) = AT_PAGESZ;
	*(buf + i + 3) = 0x1000;

	*(buf + i + 4) = AT_PHDR;
	*(buf + i + 5) = meta->phdr_addr;

	*(buf + i + 6) = AT_PHENT;
	*(buf + i + 7) = meta->phentsize;

	*(buf + i + 8) = AT_PHNUM;
	*(buf + i + 9) = meta->phnum;

	*(buf + i + 10) = AT_FLAGS;
	*(buf + i + 11) = meta->flags;

	*(buf + i + 12) = AT_ENTRY;
	*(buf + i + 13) = meta->entry;

	*(buf + i + 14) = AT_UID;
	*(buf + i + 15) = 1000;

	*(buf + i + 16) = AT_EUID;
	*(buf + i + 17) = 1000;

	*(buf + i + 18) = AT_GID;
	*(buf + i + 19) = 1000;

	*(buf + i + 20) = AT_EGID;
	*(buf + i + 21) = 1000;

	*(buf + i + 22) = AT_CLKTCK;
	*(buf + i + 23) = 100;

	*(buf + i + 24) = AT_HWCAP;
	*(buf + i + 25) = 0;

	*(buf + i + 26) = AT_PLATFORM;
	*(buf + i + 27) = top_vaddr - 64 * 2;

	*(buf + i + 28) = AT_RANDOM;
	*(buf + i + 29) = top_vaddr - 64;	/* random 16 bytes */

	*(buf + i + 30) = AT_NULL;
	*(buf + i + 31) = 0;

	/* add more auxv here */
}
