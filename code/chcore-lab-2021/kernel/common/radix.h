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

#define RADIX_NODE_BITS (9)
#define RADIX_NODE_SIZE (1 << (RADIX_NODE_BITS))
#define RADIX_NODE_MASK (RADIX_NODE_SIZE - 1)
#define RADIX_MAX_BITS (64)

struct radix_node {
	union {
		struct radix_node *children[RADIX_NODE_SIZE];
		void *values[RADIX_NODE_SIZE];
	};
};
struct radix {
	struct radix_node *root;
	void (*value_deleter) (void *);
};

/* interfaces */
struct radix *new_radix(void);
void init_radix(struct radix *radix);
int radix_add(struct radix *radix, u64 key, void *value);
void *radix_get(struct radix *radix, u64 key);
int radix_free(struct radix *radix);
int radix_del(struct radix *radix, u64 key);

void init_radix_w_deleter(struct radix *radix, void (*value_deleter) (void *));
