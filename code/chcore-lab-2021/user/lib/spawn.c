#include <lib/defs.h>
#include <lib/errno.h>
#include <lib/launcher.h>
#include <lib/print.h>
#include <lib/proc.h>
#include <lib/syscall.h>

/* main thread info */
struct mt_info
{
	u64 stack_base;
	u64 stack_size;
	u64 prio;
};

/* An example to launch an (libc) application process */

#define AT_NULL 0	   /* end of vector */
#define AT_IGNORE 1	   /* entry should be ignored */
#define AT_EXECFD 2	   /* file descriptor of program */
#define AT_PHDR 3	   /* program headers for program */
#define AT_PHENT 4	   /* size of program header entry */
#define AT_PHNUM 5	   /* number of program headers */
#define AT_PAGESZ 6	   /* system page size */
#define AT_BASE 7	   /* base address of interpreter */
#define AT_FLAGS 8	   /* flags */
#define AT_ENTRY 9	   /* entry point of program */
#define AT_NOTELF 10   /* program is not ELF */
#define AT_UID 11	   /* real uid */
#define AT_EUID 12	   /* effective uid */
#define AT_GID 13	   /* real gid */
#define AT_EGID 14	   /* effective gid */
#define AT_PLATFORM 15 /* string identifying CPU for optimizations */
#define AT_HWCAP 16	   /* arch dependent hints at CPU capabilities */
#define AT_CLKTCK 17   /* frequency at which times() increments */
/* AT_* values 18 through 22 are reserved */
#define AT_SECURE 23 /* secure mode boolean */
#define AT_BASE_PLATFORM                                      \
	24				 /* string identifying real platform, may \
					  * differ from AT_PLATFORM. */
#define AT_RANDOM 25 /* address of 16 random bytes */
#define AT_HWCAP2 26 /* extension of AT_HWCAP */

#define AT_EXECFN 31 /* filename of program */

#define NO_AFF -1

char init_env[PAGE_SIZE];
const char PLAT[] = "aarch64";

static void construct_init_env(char *env, u64 top_vaddr,
							   struct process_metadata *meta, char *name,
							   struct pmo_map_request *pmo_map_reqs,
							   int nr_pmo_map_reqs, int caps[], int nr_caps)
{
	int i, j;
	char *name_str;
	char *plat_str;
	u64 *buf;

	int argc = 1; /* at least 1 for binary name */

	/* clear init_env */
	for (i = 0; i < PAGE_SIZE; ++i)
	{
		env[i] = 0;
	}

	/* strings */
	/* the last 64 bytes */
	name_str = env + PAGE_SIZE - 64;
	i = 0;
	while (name[i] != '\0')
	{
		name_str[i] = name[i];
		++i;
	}
	// printf("name_str: %s\n", name_str);

	/* the second last 64 bytes */
	plat_str = env + PAGE_SIZE - 2 * 64;
	i = 0;
	while (PLAT[i] != '\0')
	{
		plat_str[i] = PLAT[i];
		++i;
	}
	// printf("plat_str: %s\n", plat_str);

	buf = (u64 *)env;
	/* argc */
	*buf = argc;

	/* argv */
	*(buf + 1) = top_vaddr - 64;
	/* add more argv here */
	*(buf + 2) = (u64)NULL;

	/* envp */
	i = 3;
	/*
	 * e.g.,
	 * info_page_vaddr = (u64)envp[0];
	 * fs_server_cap = (u64)envp[1];
	 */
	if (nr_pmo_map_reqs == 0)
	{
		/* set info_page_vaddr to 0x0 */
		*(buf + i) = (u64)0;
		i = i + 1;
	}
	else
	{
		for (j = 0; j < nr_pmo_map_reqs; ++j)
		{
			*(buf + i + j) = (u64)pmo_map_reqs[j].addr;
		}
		i = i + j;
	}
	for (j = 0; j < nr_caps; ++j)
	{
		/* e.g., fs_server_cap = (u64)envp[1]; */
		*(buf + i + j) = (u64)caps[j];
	}
	i = i + j;
	/* add more envp here */
	*(buf + i) = (u64)NULL;

	/* auxv */
	i = i + 1;
	*(buf + i + 0) = AT_SECURE;
	*(buf + i + 1) = 0;

	*(buf + i + 2) = AT_PAGESZ;
	*(buf + i + 3) = PAGE_SIZE;

	*(buf + i + 4) = AT_PHDR;
	*(buf + i + 5) = meta->phdr_addr;

	// printf("phdr_addr is 0x%lx\n", meta->phdr_addr);

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

	*(buf + i + 25) = AT_PLATFORM;
	*(buf + i + 26) = top_vaddr - 64 * 2;

	*(buf + i + 27) = AT_RANDOM;
	*(buf + i + 28) = top_vaddr - 64; /* random 16 bytes */

	*(buf + i + 29) = AT_NULL;
	*(buf + i + 30) = 0;

	/* add more auxv here */
}

#define MAX_NR_CAPS 16
/*
 * Lab4 - exercise 14/15
 * The core function used by spawn
 * @ user_elf: elf struct 要执行的特定文件的ELF
 * @ child_process_cap: if not NULL, set to child_process_cap that can be
 *                    used in current process.
 *						用于输出所创建子进程capability
 * @ child_main_thread_cap: if not NULL, set to child_main_thread_cap
 *                        that can be used in current process.
 *							用于输出子进程主线程的capability
 * @ pmo_map_reqs：用于指定父进程与子进程共享内存的映射
 * @ nr_pmo_map_reqs: input pmos to map in the new process
 *					指定pmo_map_reqs的数量		
 * @ caps：用于指定父进程需要转移给子进程的capability
 * @ nr_caps: copy from farther process to child process 指定caps的数量
 * @ aff: affinity 是子进程的主线程的亲和性
 *
 */
#define LAB4_SPAWN_BLANK 0
int launch_process_with_pmos_caps(struct user_elf *user_elf,
								  int *child_process_cap,
								  int *child_main_thread_cap,
								  struct pmo_map_request *pmo_map_reqs,
								  int nr_pmo_map_reqs, int caps[], int nr_caps,
								  s32 aff)
{
	int new_process_cap;
	int main_thread_cap;
	int ret;

	long pc;

	/* for creating pmos */
	struct pmo_request pmo_requests[1];
	int main_stack_cap;
	u64 stack_offset;
	u64 stack_top;
	u64 stack_va;
	u64 p_vaddr;
	int i;
	/* for mapping pmos */
	struct pmo_map_request pmo_map_requests[1];
	int transfer_caps[MAX_NR_CAPS];
	// Lab4 useless code, help avoid compile warning, you can delete it as you wish
	transfer_caps[0] = 0;

	{
		/**
		 *  Step 1: create a new process with an empty vmspace 
		 *  You do not need to modify code in this scope
		 */
		new_process_cap = usys_create_process();
		if (new_process_cap < 0)
		{
			printf("%s: fail to create new_process_cap (ret: %d)\n",
				   __func__, new_process_cap);
			goto fail;
		}
	}

	{
		/**
		 *  Step 2: 
		 *  Map each segment in the elf binary to child process
		 *  You do not need to modify code in this scope
		 */
		for (i = 0; i < 2; ++i)
		{
			p_vaddr = user_elf->user_elf_seg[i].p_vaddr;
			ret = usys_map_pmo(new_process_cap,
							   user_elf->user_elf_seg[i].elf_pmo,
							   ROUND_DOWN(p_vaddr, PAGE_SIZE),
							   user_elf->user_elf_seg[i].flags);

			if (ret < 0)
			{
				printf("usys_map_pmo ret %d\n", ret);
				usys_exit(-1);
			}
		}
		pc = user_elf->elf_meta.entry;
	}

	{
		/** 
		 * Step 3:
		 * create pmo for the stack of main thread stack in current
		 * process.
		 *  You do not need to modify code in this scope
		 */

		/* 
		 * 此处pmo_requests是用来创建main_stack_pmo的 
		 * 大小为MAIN_THREAD_STACK_SIZE
		 * 应立即分配 : #define PMO_DATA      immediate allocation 
		 */
		pmo_requests[0].size = MAIN_THREAD_STACK_SIZE;
		pmo_requests[0].type = PMO_DATA;

		ret = usys_create_pmos((void *)pmo_requests, 1);
		if (ret != 0)
		{
			printf("%s: fail to create_pmos (ret: %d)\n", __func__,
				   ret);
			goto fail;
		}

		/* get result caps */
		main_stack_cap = pmo_requests[0].ret_cap;
		if (main_stack_cap < 0)
		{
			printf("%s: fail to create_pmos (ret: %d)\n", __func__,
				   ret);
			goto fail;
		}
	}

	{
		/**
		 * Step A :
		 * Transfer the capbilities (nr_caps) of current process to the
		 * capbilities of child process
		 */

		if (nr_caps > 0)
		{
			/* usys_transfer_caps is used during process creation */
			ret = usys_transfer_caps(new_process_cap, caps, nr_caps,
									 transfer_caps);
			if (ret != 0)
			{
				printf("usys_transfer_caps ret %d\n", ret);
				usys_exit(-1);
			}
		}
	}

	{
		/**
		 * Step B :
		 * Use the given pmo_map_reqs to map the vmspace in the child
		 * process
		 */
		if (nr_pmo_map_reqs > 0)
		{
			ret =
				usys_map_pmos(new_process_cap, (void *)pmo_map_reqs,
							  nr_pmo_map_reqs);
			if (ret != 0)
			{
				printf("%s: fail to map_pmos (ret: %d)\n",
					   __func__, ret);
				goto fail;
			}
		}
	}

	{
		/**
		 * Step 4
		 * Prepare the arguments in the top page of the main thread's
		 * stack.
		 * You should calculate the correct value of **stack_top** and
		 * **stack_offset**
		 * 
		 * 写了初始页面，就是写了argc,argv这些，大小为PAGE_SIZE
		 * 栈从这一页下面开始
		 */

		/**
		 * Hints: refer to <defs.h>
		 * For stack_top, what's the virtual address of top of the main
		 * thread's stack?
		 *
		 * For stack_offset, when the main thread gets
		 * to execute the first time, what's the virtual adress the sp
		 * register points to?
		 * stack_offset is the offset from main thread's stack base to
		 * that address.
		 */
		stack_top = MAIN_THREAD_STACK_BASE + MAIN_THREAD_STACK_SIZE; /* 栈顶 = 栈底(栈最多可以往下涨到的限度地址) + 栈的大小*/
		stack_offset = MAIN_THREAD_STACK_SIZE - PAGE_SIZE;			 /* stack_offset的意思是，栈顶距离pmo->start多少开始 见图可知*/

		/* Construct the parameters on the top page of the stack */
		construct_init_env(init_env, stack_top, &user_elf->elf_meta,
						   user_elf->path, pmo_map_reqs,
						   nr_pmo_map_reqs, transfer_caps, nr_caps);
	}

	{
		/**
		 * Step 5
		 * Update the main thread stack's pmo
		 * You only need to write the modified page
		 * 将初始页面写到main_stack_pmo的顶端页
		 */
		/*
		 * 把init_env拷到pmo->start + stack_offset,大小为PAGE_SIZE
		 * (那个图 pmo->start是在最下面)
		 */
		ret = usys_write_pmo(main_stack_cap, stack_offset, init_env,
							 PAGE_SIZE);
		if (ret != 0)
		{
			printf("%s: fail to write_pmo (ret: %d)\n", __func__,
				   ret);
			goto fail;
		}
	}

	{
		/**
		 * Step 6
		 *  map the the main thread stack's pmo in the new process.
		 *  Both VM_READ and VM_WRITE permission should be set.
		 * 
		 * 使用sys_write_pmo()将main_stack_pmo映射到子进程。
		 * 应该将其映射到具有VM_READ和VM_WRITE权限的子进程的地址MAIN_THREAD_STACK_BASE。
		 */
		pmo_map_requests[0].pmo_cap = main_stack_cap;
		pmo_map_requests[0].addr = MAIN_THREAD_STACK_BASE;
		pmo_map_requests[0].perm = VM_READ | VM_WRITE;

		ret =
			usys_map_pmos(new_process_cap, (void *)pmo_map_requests, 1);

		if (ret != 0)
		{
			printf("%s: fail to map_pmos (ret: %d)\n", __func__,
				   ret);
			goto fail;
		}
	}

	{
		/**
		 * Step 7
		 * create main thread in the new process.
		 * Please fill the stack_va!
		 */
		stack_va = (MAIN_THREAD_STACK_BASE + MAIN_THREAD_STACK_SIZE) - PAGE_SIZE;
		main_thread_cap =
			usys_create_thread(new_process_cap, stack_va, pc,
							   (u64)NULL, MAIN_THREAD_PRIO, aff);
		if (main_thread_cap < 0)
		{
			printf("%s: fail to create thread (ret: %d)\n",
				   __func__, ret);
			goto fail;
		}
	}

	{
		/* Step C: Output the child process & thread capabilities */
		if (child_process_cap)
		{
			*child_process_cap = new_process_cap;
		}
		if (child_main_thread_cap)
		{
			*child_main_thread_cap = main_thread_cap;
		}
	}

	return 0;
fail:
	return -EINVAL;
}

int spawn(char *path, int *new_process_cap, int *new_thread_cap,
		  struct pmo_map_request *pmo_map_reqs, int nr_pmo_map_reqs, int caps[],
		  int nr_caps, int aff)
{
	struct user_elf user_elf;
	int ret;

	ret = readelf_from_kernel_cpio(path, &user_elf);
	if (ret < 0)
	{
		printf("[Client] Cannot create server.\n");
		return ret;
	}

	return launch_process_with_pmos_caps(&user_elf, new_process_cap,
										 new_thread_cap, pmo_map_reqs,
										 nr_pmo_map_reqs, caps, nr_caps,
										 aff);
}
