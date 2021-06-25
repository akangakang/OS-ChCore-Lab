#pragma once

#include <lib/type.h>

struct pmo_request {
	/* input: args */
	u64 size;
	u64 type;

	/* output: return value */
	u64 ret_cap;
};

struct pmo_map_request {
	/* input: args */
	u64 pmo_cap;
	u64 addr;
	u64 perm;

	/* output: return value */
	u64 ret;
};

int launch_process(struct user_elf *user_elf,
		   int *child_process_cap,
		   int *child_main_thread_cap,
		   struct pmo_map_request *pmo_map_reqs,
		   int nr_pmo_map_reqs, int caps[], int nr_caps, int cpuid);

int launch_process_with_pmos_caps(struct user_elf *user_elf,
				  int *child_process_cap,
				  int *child_main_thread_cap,
				  struct pmo_map_request *pmo_map_reqs,
				  int nr_pmo_map_reqs, int caps[], int nr_caps,
				  s32 aff);
